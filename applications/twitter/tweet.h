// ------------------------------------------------------------
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//

#ifndef MOCK_KEY_VALUE_STORE_TWEET_H
#define MOCK_KEY_VALUE_STORE_TWEET_H

#include <string>

class tweet {
public:
    tweet(long tweet_id, std::string content, long timestamp);
    std::string get_tweet();
    long get_id();
    long get_timestamp() const;
    long get_likes();
    long get_retweets();
    void set_likes(long likes);
    void set_retweets(long retweets);
    void increment_like_count();
    void increment_retweet_count();

private:
    long tweet_id;
    std::string content;
    long timestamp;
    long like_count, retweet_count;
};

tweet::tweet(long tweet_id, std::string content, long timestamp) {
    this->tweet_id = tweet_id;
    this->content = content;
    this->timestamp = timestamp;
    like_count = retweet_count = 0;
}

std::string tweet::get_tweet() {
    return content;
}

long tweet::get_id() {
    return tweet_id;
}

long tweet::get_timestamp() const {
    return timestamp;
}

long tweet::get_likes() {
    return like_count;
}

long tweet::get_retweets() {
    return retweet_count;
}

void tweet::set_likes(long likes) {
    this->like_count = likes;
}

void tweet::set_retweets(long retweets) {
    this->retweet_count = retweets;
}

void tweet::increment_like_count() {
    like_count++;
}

void tweet::increment_retweet_count() {
    retweet_count++;
}

#endif //MOCK_KEY_VALUE_STORE_TWEET_H
