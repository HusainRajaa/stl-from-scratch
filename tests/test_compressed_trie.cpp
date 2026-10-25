#include "test_runner.hpp"
#include <stl/compressed_trie.hpp>
#include <algorithm>
#include <string>
#include <vector>

TEST_CASE(CompressedTrieTest, BasicInsertAndContains) {
    stl::compressed_trie trie;
    ASSERT_TRUE(trie.empty());
    ASSERT_EQ(trie.size(), 0);

    ASSERT_TRUE(trie.insert("apple"));
    ASSERT_TRUE(trie.insert("app"));
    ASSERT_TRUE(trie.insert("application"));
    ASSERT_TRUE(trie.insert("banana"));

    // Duplicate inserts
    ASSERT_FALSE(trie.insert("apple"));
    ASSERT_FALSE(trie.insert("app"));

    ASSERT_EQ(trie.size(), 4);
    ASSERT_FALSE(trie.empty());

    ASSERT_TRUE(trie.contains("apple"));
    ASSERT_TRUE(trie.contains("app"));
    ASSERT_TRUE(trie.contains("application"));
    ASSERT_TRUE(trie.contains("banana"));

    ASSERT_FALSE(trie.contains("ap"));
    ASSERT_FALSE(trie.contains("appl"));
    ASSERT_FALSE(trie.contains("orange"));
    ASSERT_FALSE(trie.contains(""));
}

TEST_CASE(CompressedTrieTest, EmptyStringKey) {
    stl::compressed_trie trie;
    ASSERT_FALSE(trie.contains(""));

    ASSERT_TRUE(trie.insert(""));
    ASSERT_TRUE(trie.contains(""));
    ASSERT_EQ(trie.size(), 1);

    ASSERT_FALSE(trie.insert(""));
    ASSERT_TRUE(trie.erase(""));
    ASSERT_FALSE(trie.contains(""));
    ASSERT_EQ(trie.size(), 0);
}

TEST_CASE(CompressedTrieTest, PrefixMatching) {
    stl::compressed_trie trie{"cat", "caterpillar", "dog", "door", "dormitory"};

    ASSERT_TRUE(trie.starts_with("cat"));
    ASSERT_TRUE(trie.starts_with("cater"));
    ASSERT_TRUE(trie.starts_with("d"));
    ASSERT_TRUE(trie.starts_with("do"));
    ASSERT_TRUE(trie.starts_with("dorm"));

    ASSERT_FALSE(trie.starts_with("cattle"));
    ASSERT_FALSE(trie.starts_with("dot"));
    ASSERT_FALSE(trie.starts_with("z"));
}

TEST_CASE(CompressedTrieTest, Autocomplete) {
    stl::compressed_trie trie{
        "card", "care", "careful", "careless", "cars", "cart", "carve", "dog"
    };

    auto res = trie.autocomplete("car");
    std::vector<std::string> expected = {
        "card", "care", "careful", "careless", "cars", "cart", "carve"
    };
    ASSERT_EQ(res.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i) {
        ASSERT_EQ(res[i], expected[i]);
    }

    // Limit results
    auto limited = trie.autocomplete("care", 2);
    ASSERT_EQ(limited.size(), 2);
    ASSERT_EQ(limited[0], "care");
    ASSERT_EQ(limited[1], "careful");

    // Empty prefix gives all words in lexicographical order
    auto all_words = trie.autocomplete("");
    ASSERT_EQ(all_words.size(), 8);
    ASSERT_TRUE(std::is_sorted(all_words.begin(), all_words.end()));

    // Non-existent prefix
    auto empty_res = trie.autocomplete("zebra");
    ASSERT_TRUE(empty_res.empty());
}

TEST_CASE(CompressedTrieTest, LongestCommonPrefix) {
    stl::compressed_trie trie{"/usr", "/usr/bin", "/usr/local/bin", "/var/log"};

    ASSERT_EQ(trie.longest_common_prefix("/usr/bin/bash"), "/usr/bin");
    ASSERT_EQ(trie.longest_common_prefix("/usr/local/bin/python"), "/usr/local/bin");
    ASSERT_EQ(trie.longest_common_prefix("/usr/lib"), "/usr");
    ASSERT_EQ(trie.longest_common_prefix("/etc/passwd"), "");
}

TEST_CASE(CompressedTrieTest, EraseAndRecompression) {
    stl::compressed_trie trie{"tester", "test", "testing", "team"};

    ASSERT_EQ(trie.size(), 4);
    ASSERT_TRUE(trie.erase("test"));
    ASSERT_EQ(trie.size(), 3);
    ASSERT_FALSE(trie.contains("test"));
    ASSERT_TRUE(trie.contains("tester"));
    ASSERT_TRUE(trie.contains("testing"));
    ASSERT_TRUE(trie.contains("team"));

    ASSERT_TRUE(trie.erase("tester"));
    ASSERT_TRUE(trie.erase("testing"));
    ASSERT_EQ(trie.size(), 1);
    ASSERT_TRUE(trie.contains("team"));

    ASSERT_FALSE(trie.erase("not_found"));
    ASSERT_TRUE(trie.erase("team"));
    ASSERT_TRUE(trie.empty());
}

TEST_CASE(CompressedTrieTest, CopyAndMove) {
    stl::compressed_trie t1{"alpha", "beta", "gamma"};
    stl::compressed_trie t2 = t1; // Copy

    ASSERT_EQ(t2.size(), 3);
    ASSERT_TRUE(t2.contains("alpha"));
    ASSERT_TRUE(t2.contains("beta"));
    ASSERT_TRUE(t2.contains("gamma"));

    stl::compressed_trie t3 = std::move(t1); // Move
    ASSERT_EQ(t3.size(), 3);
    ASSERT_TRUE(t1.empty());
}

int main() {
    return RUN_ALL_TESTS();
}
