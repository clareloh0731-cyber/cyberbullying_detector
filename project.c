#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_WORDS 10000
#define MAX_WORD_LENGTH 50
#define MAX_STOPWORDS 500
#define MAX_TEXT_LENGTH 50000  

struct WordInfo {
    char word[MAX_WORD_LENGTH];
    int count;
};

// Load stopwords list
int load_stopwords(char stopwords[][MAX_WORD_LENGTH]) {
    FILE *file = fopen("stopwords.txt", "r");
    if (file == NULL) {
        printf("Error: Cannot open stopwords.txt\n");
        return 0;
    }
    
    int count = 0;
    char line[MAX_WORD_LENGTH];
    
    while (fgets(line, sizeof(line), file) && count < MAX_STOPWORDS) {
        // Remove newline character
        line[strcspn(line, "\n")] = '\0';
        // Convert to lowercase
        for(int i = 0; line[i]; i++) {
            line[i] = tolower(line[i]);
        }
        if(strlen(line) > 0) {
            strcpy(stopwords[count], line);
            count++;
        }
    }
    
    fclose(file);
    printf("Successfully loaded %d stopwords\n", count);
    return count;
}

// Check if a word is a stopword
int is_stopword(char *word, char stopwords[][MAX_WORD_LENGTH], int stop_count) {
    for (int i = 0; i < stop_count; i++) {
        if (strcmp(word, stopwords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Tokenize and clean text - SAFER VERSION
int tokenize_and_clean(char *text, struct WordInfo words[], char stopwords[][MAX_WORD_LENGTH], int stop_count, int *total_chars) {
    int word_count = 0;
    char *token;
    const char *delimiters = " .,!?;:\"\'()[]{}@#\n\t\r";
    
    printf("Text length: %lu characters\n", strlen(text));
    
    // Create a copy of text with size limit
    char *text_copy = malloc(strlen(text) + 1);
    if (text_copy == NULL) {
        printf("Error: Memory allocation failed\n");
        return 0;
    }
    strcpy(text_copy, text);
    
    token = strtok(text_copy, delimiters);
    int processed_tokens = 0;
    
    while (token != NULL && word_count < MAX_WORDS && processed_tokens < 10000) {
        processed_tokens++;
        
        // Clean word: convert to lowercase
        char clean_word[MAX_WORD_LENGTH];
        if(strlen(token) >= MAX_WORD_LENGTH) {
            // Skip words that are too long
            token = strtok(NULL, delimiters);
            continue;
        }
        
        strcpy(clean_word, token);
        for (int i = 0; clean_word[i]; i++) {
            clean_word[i] = tolower(clean_word[i]);
        }
        
        // Only keep words containing letters
        int has_letters = 0;
        for (int i = 0; clean_word[i]; i++) {
            if (isalpha(clean_word[i])) {
                has_letters = 1;
                break;
            }
        }
        
        if (has_letters && strlen(clean_word) > 0 && !is_stopword(clean_word, stopwords, stop_count)) {
            // Check if word already exists
            int found = 0;
            for (int i = 0; i < word_count; i++) {
                if (strcmp(words[i].word, clean_word) == 0) {
                    words[i].count++;
                    found = 1;
                    break;
                }
            }
            
            // If new word, add to array
            if (!found && word_count < MAX_WORDS) {
                strncpy(words[word_count].word, clean_word, MAX_WORD_LENGTH - 1);
                words[word_count].word[MAX_WORD_LENGTH - 1] = '\0'; // Ensure null termination
                words[word_count].count = 1;
                word_count++;
            }
            
            // Count characters
            *total_chars += strlen(clean_word);
        }
        
        token = strtok(NULL, delimiters);
    }
    
    free(text_copy); // Free allocated memory
    printf("Processed %d tokens, found %d unique words\n", processed_tokens, word_count);
    return word_count;
}

// Count number of sentences
int count_sentences(char *text) {
    int sentences = 0;
    for (int i = 0; text[i] && i < MAX_TEXT_LENGTH; i++) {
        if (text[i] == '.' || text[i] == '!' || text[i] == '?') {
            sentences++;
            // Skip consecutive punctuation
            while (text[i+1] == '.' || text[i+1] == '!' || text[i+1] == '?') {
                i++;
            }
        }
    }
    return sentences > 0 ? sentences : 1;
}

// Generate statistics report
void generate_statistics(struct WordInfo words[], int word_count, int total_words, int total_chars, int sentences, int stopwords_removed) {
    printf("\n=== TEXT ANALYSIS REPORT ===\n");
    printf("Total words: %d\n", total_words);
    printf("Unique words: %d\n", word_count);
    printf("Number of sentences: %d\n", sentences);
    
    if (sentences > 0) {
        printf("Average sentence length: %.1f words\n", (float)total_words / sentences);
    }
    
    printf("Total characters: %d\n", total_chars);
    
    if (total_words > 0) {
        printf("Average word length: %.1f characters\n", (float)total_chars / total_words);
    }
    
    printf("Stopwords filtered: %d\n", stopwords_removed);
    
    if (total_words > 0) {
        float lexical_diversity = (float)word_count / total_words;
        printf("Lexical diversity: %.3f\n", lexical_diversity);
    }
}

// Bubble sort by frequency
void sort_by_frequency(struct WordInfo words[], int count) {
    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            if (words[j].count < words[j+1].count) {
                // Swap
                struct WordInfo temp = words[j];
                words[j] = words[j+1];
                words[j+1] = temp;
            }
        }
    }
}

// Display top N frequent words
void show_top_words(struct WordInfo words[], int word_count, int n) {
    if (n > word_count) {
        n = word_count;
    }
    
    printf("\nTop %d most frequent words:\n", n);
    for (int i = 0; i < n; i++) {
        printf("%d. %s (%d occurrences)\n", i + 1, words[i].word, words[i].count);
    }
}

// Main function
int main() {
    printf("=== Part 2 Text Analyzer ===\n");
    
    // Load stopwords
    char stopwords[MAX_STOPWORDS][MAX_WORD_LENGTH];
    int stop_count = load_stopwords(stopwords);
    if (stop_count == 0) {
        printf("Cannot continue: Stopwords file failed to load\n");
        return 1;
    }
    
    // Open input file
    FILE *input_file = fopen("input.txt", "r");
    if (input_file == NULL) {
        printf("ERROR: Cannot open input.txt\n");
        return 1;
    }
    
    // Read file content with size limit
    char text[MAX_TEXT_LENGTH] = "";
    char line[1000];
    int total_words_raw = 0;
    int stopwords_removed = 0;
    int total_chars = 0;
    
    printf("Reading file content...\n");
    while (fgets(line, sizeof(line), input_file) && strlen(text) < MAX_TEXT_LENGTH - 1000) {
        strcat(text, line);
        
        // Count raw words
        char line_copy[1000];
        strcpy(line_copy, line);
        char *token = strtok(line_copy, " .,!?;:\"\'()[]{}@#\n\t\r");
        while (token) {
            total_words_raw++;
            token = strtok(NULL, " .,!?;:\"\'()[]{}@#\n\t\r");
        }
    }
    fclose(input_file);
    
    printf("File reading completed. Total raw words: %d\n", total_words_raw);
    printf("Text buffer used: %lu/%d characters\n", strlen(text), MAX_TEXT_LENGTH);
    
    if (strlen(text) == 0) {
        printf("ERROR: No content read from file\n");
        return 1;
    }
    
    // Process text
    struct WordInfo *words = malloc(MAX_WORDS * sizeof(struct WordInfo));
    if (words == NULL) {
        printf("Error: Could not allocate memory for words array\n");
        return 1;
    }
    
    printf("Starting text processing...\n");
    int word_count = tokenize_and_clean(text, words, stopwords, stop_count, &total_chars);
    
    // Calculate total words after processing
    int total_words_clean = 0;
    for (int i = 0; i < word_count; i++) {
        total_words_clean += words[i].count;
    }
    
    stopwords_removed = total_words_raw - total_words_clean;
    
    // Count sentences
    int sentences = count_sentences(text);
    
    // Generate statistics report
    generate_statistics(words, word_count, total_words_clean, total_chars, sentences, stopwords_removed);
    
    // Sort and show frequent words
    sort_by_frequency(words, word_count);
    show_top_words(words, word_count, 10);
    
    // Free allocated memory
    free(words);
    
    printf("\nAnalysis completed successfully!\n");
    return 0;
}