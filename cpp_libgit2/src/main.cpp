//#include <bits/stdc++.h>

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <string>
#include <cstring>

#include <git2.h>

using namespace std;

/*======UTILITY======*/

//#define DEBUG

#ifdef DEBUG
#define perr(err) { cout << __LINE__ << ": " << err << endl; }
#else
#define perr(err) { int error = err; if (error) cout << "errror: libgit2: " << __LINE__ << ": " << error << endl; }
#endif

/*======DECOR======*/

void print_nyi(bool ex = true)
{
    cout << "Not yet implemented" << endl;
    if (ex)
        exit(3);
}

void print_err(const string &err, bool ex = true)
{
    cout << err << endl;
    if (ex)
        exit(-1);
}

const char usage[] = 
"Usage: git-reword one (-d | -m) <rev> <new_message>\n"
"       git-reword list (-d | -m) (-s | -c | -n) <path_to_list-file>\n"
"    \n"
"Commands \"one\" & \"list\":\n"
"    \n"
"    -d  (not yet implemented)\n"
"        Delete merge-commits on the edited path.\n"
"    \n"
"    -m\n"
"        Save merge-commits on the edited path.\n"
"\n"
"Command \"list\":\n"
"    \n"
"    -s (not yet implemented)\n"
"        Automatically format the list because it does not satisfy some conditions.\n"
"    \n"
"    -c (not yet implemented)\n"
"        Check that the list satisfies the conditions.\n"
"    \n"
"    -n\n"
"        The list fully meets all the requirements, the user knows what he is doing.\n"
"\n"
"    list-file:\n"
"\n"
"        format:\n"
"            <rev_1> <new_message_1>\n"
"            <rev_2> <new_message_2>\n"
"            ...\n"
"            <rev_n> <new_message_n>\n"
"        \n"
"        The list must satisfy certain conditions, which depend on the version.";

void print_usage(bool ex = true)
{
    cout << usage << endl;
    if (ex)
        exit(2);
}

/*======MY-GIT======*/

string mgit_oid2str(git_oid oid)
{
    char buf[GIT_OID_HEXSZ + 1];
    git_oid_tostr(buf, sizeof(buf), &oid);
    return string(buf);
}

git_oid mgit_str2oid(const string &str)
{
    git_oid oid;
    git_oid_fromstr(&oid, str.c_str());
    return oid;
}

git_oid mgit_rev2oid(git_repository *repo, const string &rev)
{
    git_object *object = NULL;
    git_revparse_single(&object, repo, rev.c_str());
    git_oid oid = *git_object_id(object);
    return oid;
}

string mgit_rev2str(git_repository *repo, const string &rev)
{
    return mgit_oid2str(mgit_rev2oid(repo, rev));
}

void mgit_commit_message(git_commit *commit, string &msg, string &msg_enc)
{
    const char *encoding = git_commit_message_encoding(commit);
    const char *message = git_commit_message_raw(commit);
    msg = string(message);
    msg_enc = string(encoding == NULL ? "" : encoding);
}

/*======WORK======*/

const int PARENTS_CNT_MAX = 100;

git_repository *repo = NULL;
git_odb *odb = NULL;

git_commit *parents_commit[PARENTS_CNT_MAX];

void workx_line(git_oid s_oid, const map<string, string> &msg_dict, bool del_merge)
{
#ifdef DEBUG
    //print(msg_dict);
#endif
    
    /// libgit2: init revwalk
    git_revwalk *walker = NULL;
    perr(git_revwalk_new(&walker, repo));
    string s_str = mgit_oid2str(s_oid);
    string range_str = s_str + string("~..HEAD");
    perr(git_revwalk_push_range(walker, range_str.c_str()));
    git_revwalk_sorting(walker, GIT_SORT_REVERSE);
    
    /// libgit2: revwalk
    map<string, string> str2newstr;
    git_oid oid, last_oid;
#ifdef DEBUG
    cout << "======================" << endl;
#endif
    while (!git_revwalk_next(&oid, walker)) {
        string str = mgit_oid2str(oid);
        
        /// get commit
        git_commit *commit = NULL;
        perr(git_commit_lookup(&commit, repo, &oid));
        
        /// get tree_oid
        const git_oid *tree_oid = git_commit_tree_id(commit);
        git_tree *tree = NULL;
        perr(git_tree_lookup(&tree, repo, tree_oid));
        
        /// get parents_commit
        vector<string> parents_str;
        size_t parents_cnt = git_commit_parentcount(commit);
        for (size_t i = 0; i < parents_cnt; ++i) {
            const git_oid *parent_oid = git_commit_parent_id(commit, i);
            string parent_str = mgit_oid2str(*parent_oid);
            if (str2newstr.find(parent_str) != str2newstr.end()) {
                parents_str.push_back(str2newstr[parent_str]);
                git_oid parent_oid_new = mgit_str2oid(parents_str.back());
                git_commit *parent_commit_new = NULL;
                perr(git_commit_lookup(&parent_commit_new, repo, &parent_oid_new));
                parents_commit[i] = parent_commit_new;
            } else {
                parents_str.push_back(parent_str);
                perr(git_commit_parent(&parents_commit[i], commit, i));
            }
        }
        
        /// get message
        string msg_new, msg_enc_new;
        if (msg_dict.find(str) != msg_dict.end())
            msg_new = msg_dict.at(str);
        else
            mgit_commit_message(commit, msg_new, msg_enc_new);
        
        /// read data (author, commiter)
        git_odb_object *odb_object = NULL;
        git_odb_read(&odb_object, odb, &oid);
        string data_old((const char*)git_odb_object_data(odb_object));
        istringstream iss(data_old);
        string author, committer;
        for (string line; getline(iss, line); ) {
            if (line.find("author ") == 0)
                author = line;
            else if (line.find("committer ") == 0)
                committer = line;
        }
        
        /// generate data
        string data;
        data += "tree " + mgit_oid2str(*tree_oid) + "\n";
        data += "parent";
        for (const string &parent_str : parents_str) {
            data += " ";
            data += parent_str;
        }
        data += "\n";
        data += author + "\n";
        data += committer + "\n";
        if (!msg_enc_new.empty())
            data += msg_enc_new + "\n";
        data += "\n";
        data += msg_new;
        
        /// write data
        git_oid new_oid;
        git_odb_write(&new_oid, odb, data.c_str(), (int)data.size(), GIT_OBJ_COMMIT);
        
        last_oid = new_oid;
        
        string str_new = mgit_oid2str(new_oid);
        str2newstr[str] = str_new;
        
#ifdef DEBUG
        cout << "" << str << endl;
        cout << data << endl;
        cout << str_new << endl;
        cout << "----------------------" << endl;
#endif
    }
#ifdef DEBUG
    cout << "======================" << endl;
#endif
    
    /// ligti2: reset
    git_commit *last_commit = NULL;
    perr(git_commit_lookup(&last_commit, repo, &last_oid));
    git_reset(repo, (git_object *)last_commit, GIT_RESET_HARD, NULL);
}

void work_single(const string &rev, const string &msg, bool del_merge)
{
    if (del_merge)
        print_nyi(); //TODO
    
    /*if (git_get_commit_type(id) != "commit")
        print_err("error: input: it's not a commit?");*/
    
    /// libgit2: init
    git_libgit2_init();
    perr(git_repository_open(&repo, "."));
    git_repository_odb(&odb, repo);
    
    /// libgit2: resolve rev
    git_oid oid = mgit_rev2oid(repo, rev);
    
    workx_line(oid, {{mgit_oid2str(oid), msg + "\n"}}, del_merge);
    
    /// libgit2: shutdown
    git_repository_free(repo);
    git_libgit2_shutdown();
}

void work_list(const string &lst_path, int sorting, bool del_merge)
{    
    if (sorting != 2 || del_merge)
        print_nyi(); //TODO
    
    /// libgit2: init
    git_libgit2_init();
    perr(git_repository_open(&repo, "."));
    git_repository_odb(&odb, repo);
    
    ifstream lst_file(lst_path);
    map<string, string> msg_dict;
    /*for (string line; getline(lst_file, line); ) {
        /// libgit2: resolve rev
        strign str = mgit_rev2str(repo, rev);
        msg_dict[str] = msg + "\n";
    }*/
    string rev, msg;
    string last_str;
    while (lst_file >> rev >> msg) {
        /// libgit2: resolve rev
        string str = mgit_rev2str(repo, rev);
        msg_dict[str] = msg + "\n";
        last_str = str;
    }
    
    git_oid last_oid = mgit_str2oid(last_str);
    
    workx_line(last_oid, msg_dict, del_merge);
    
    /// libgit2: shutdown
    git_repository_free(repo);
    git_libgit2_shutdown();
}

/*======MAIN======*/

int main(int argc, char *argv[]) {
    if (argc == 5 && !strcmp(argv[1], "one") && (!strcmp(argv[2], "-d") || !strcmp(argv[2], "-m"))) {
        cout << "Start" << endl;
        bool del_merge = !strcmp(argv[2], "-d");
        string rev = argv[3];
        string msg = argv[4];
#ifdef DEBUG
        cout << rev << "\t" << msg << "\t" << del_merge << endl;
#endif
        work_single(rev, msg, del_merge);
        cout << "OK" << endl;
    } else if (argc == 5 && !strcmp(argv[1], "list") && (!strcmp(argv[2], "-d") || !strcmp(argv[2], "-m")) && (!strcmp(argv[3], "-s") || !strcmp(argv[3], "-c") || !strcmp(argv[3], "-n"))) {
        cout << "Start" << endl;
        bool del_merge = !strcmp(argv[2], "-d");
        int sorting = -1;
        if (!strcmp(argv[3], "-s"))
            sorting = 0;
        else if (!strcmp(argv[3], "-c"))
            sorting = 1;
        else if (!strcmp(argv[3], "-n"))
            sorting = 2;
        string lst_path = argv[4];
#ifdef DEBUG
        cout << lst_path << "\t" << sorting << "\t" << del_merge << endl;
#endif
        work_list(lst_path, sorting, del_merge);
        cout << "OK" << endl;
    } else {
        print_usage();
    }
    
    return 0;
}
