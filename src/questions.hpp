#pragma once
#include <vector>
#include <string>

struct Question {
    std::string questionText;
    std::vector<std::string> answers;
    int correctAnswerIndex;
};

std::vector<Question> GetQuestionsVector();