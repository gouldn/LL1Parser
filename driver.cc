#include <iostream>
#include <iomanip>
#include <stack>
#include <algorithm>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
using std::cin;
using std::cout;
using std::setw;
using std::left;
using std::right;
using std::stack;
using std::find;

#include "scan.h"
#include "stdlib.h"
#include "string.h"
#include "stdlib.h"

using namespace std;
using namespace scanner;


// Initializes max_terminal and num_productions with an initial scan of the grammar file
void count_input(char* inputFile);

// Initializes vectors containing the names of each terminal/nonterminal. For termainals, strings are placed at the index
// indicated by the number provided in the grammar file, whereas nonterminals are placed sequentially starting at 1;
vector<vector<string> > init_names(char* inputFile);

// Initializes and returns a two-dimension vector containing numeric representations of each terminal/nonterminal as dictated
// by the index in nonterminal_names and terminal_names respectively. Terminals are stored as negative numbers to facilitate checking
vector<vector<int> > init_productions(char* inputFile, vector<bool> &func_generates_epsilon);

// Finds a symbol (nonterminal or terminal) and returns the index of the queried symbol in nonterminal or terminal names. Negative values represent terminals while
// positive values represent nonterminals. If not found, returns an arbitrary 999;
int find_in_symbol_names(string symbol_to_find);

// Initializes and returns two-dimensional vector containing the first sets of each non-terminal in the grammar. Indexes of each
// vector in the first dimension correspond with nonterminal indices in nonterminal_names
vector<vector<int> > init_first(vector<bool> local_generates_epsilon);

// Initializes and returns a two-dimensional vector containing the follow sets of each non-terminal in the grammar. Indexes of
// each vector in the first dimension similarly correspond with nonterminal indices in nonterminal_names
vector<vector<int> > init_follow(vector<bool> local_generates_epsilon, vector<vector<int> > local_first_sets);

// Initializes the predict sets for each production in the grammar. Used to generate the parse table and depends upon the creation of both the first and follow
// sets before being executed
vector<vector<int> > init_predict(vector<bool> local_generates_epsilon, vector<vector<int> > local_first_sets, vector<vector<int> > local_follow_sets);

// Finds if a give symbol is inside of a given array. If the element is found in the given array, then the function returns true, otherwise it returns false
bool find_symbol_in_array(vector<int> find_in, int element);

// Pushes a vector of integers into a destination vector and returns a vector containing all elements of both arrays without duplicates
vector<int> push_unique_elements(vector<int> destination, vector<int> source);

// Meta-function for generating data structures for parsing. Must be called before parse()
void generate(char* inputName);

// Meta-function for parsing an input file that is piped in at the command line
void parse();

ifstream input; // Input file stream for the input grammar
string line; // String that is used to iterate over each line of the input grammar

int max_terminal; // The largest index given with the input grammar's terminals
int num_nonterminals; // The number of nonterminals found in the given grammar
int num_productions; // The number of productions that were found in the given grammar
int start_symbol = 1; // Integer representing the start symbol for this grammar

vector<string> terminal_names; // A vector containing the names of each terminal at the index given in the input grammar
vector<string> nonterminal_names; // A vector containing the nonterminal names in sequential order given in the input grammar
vector<vector<int> > full_productions; // A two-dimensional vector containing each production with numeric representations of each symbol in each production
vector<vector<int> > first_sets; // A two-dimension vector containing each of the first sets for every nonterminal in the grammar
vector<vector<int> > follow_sets; // A two-dimensional vector containing each of the first sets for every nonterminal in the grammar
vector<vector<int> > predict_sets; // A two-dimensional vector containing the predict sets for every production in the grammar
vector<vector<int> > parse_tab; // A two-dimensional vector representing this grammar's parse table
vector<vector<int> > right_hand_sides; // The right hand sides of each production in the grammar. Ordered in reverse to facilitate the use of a stack

int main(int argc, char* argv[]) {

	generate(argv[1]); // Generate data structures

	parse(); // Parse the input
}

// Meta-function for parsing an input file that is piped in at the command line
void parse() {

	stack<int> parse_stack;

	// The next symbol at the top of the parse stack, the symbol that is expected in the input stream
    int expected_symbol;

    // The current input token. Not consumed until a match is found with the top of the parse stack
    token input_tok;

    // Initial contents of the stack
    parse_stack.push(start_symbol);

    // Initialize reading of input
    input_tok = scan();

    // Loop
    do {
    	// Expected symbol resides at the top of the parse stack. Pop will only remove the element while top() will retrieve it without removal
    	expected_symbol = parse_stack.top();

    	if(input_tok.num == 39) {
    		cout << "\nInput token \"" << input_tok.image << "\" at row: " << input_tok.line << " column: " << input_tok.column << " is an invalid token and was scanned as a tok_error\n";
    		input_tok = scan();
    	}
    	// If this expected symbol is a terminal (its numeric representation is negative - as per the generation of productions)
    	if(expected_symbol < 0) {
    		// If the input token's numeric representation is equivalent to the absolute value of the expected symbol's numeric representation (scan.h has only positive integers)

    		if(input_tok.num == abs(expected_symbol)) {
    			
    			// Then a match has been found, print appropriate message
    			cout << "Match: " << input_tok.image << "\n";
    			input_tok = scan(); // Consume the input token
    			parse_stack.pop(); // Remove the item at the top of the parse stack
    		} else {
    			cout <<"Input token \"" << input_tok.image << "\" at row: " << input_tok.line << " column: " << input_tok.column << " was not matched with expected \"" << terminal_names[abs(expected_symbol)] << "\".\n\tPopping expected symbol from the parse stack\n";
    			parse_stack.pop();
    		}
    	} // Otherwise, if the expected symbol is a non-terminal
    	else if (expected_symbol > 0) {
    		// If the parse table does not have a valid entry for this input/symbol pair, handle errors
    		if(parse_tab[abs(expected_symbol)-1][input_tok.num-1] == 0) {
    			vector<int> this_nonterminals_first = first_sets[expected_symbol];
    			vector<int> this_nonterminals_follow = follow_sets[expected_symbol];
    			cout << "There was no valid entry for input/symbol pair: (" << nonterminal_names[expected_symbol] << ", " << input_tok.image << ")\n";

    			do {
    				input_tok = scan();
    				if(find_symbol_in_array(this_nonterminals_first, input_tok.num)) {
    					cout << "\tInput token: \"" << input_tok.image << "\" was found in \"" << nonterminal_names[expected_symbol] << "\"\'s first set. Continuing with this non-terminal in place\n";
    					break;
    				} else if (find_symbol_in_array(this_nonterminals_follow, input_tok.num)) {
    					cout << "\tInput token: \"" << input_tok.image << "\" was found in \"" << nonterminal_names[expected_symbol] << "\"'s first set. Popping this non-terminal off of the parse stack\n";
    					parse_stack.pop();
    					break;
    				}
    			} while(input_tok.num != tok_eof);
    		} else {
    			// If there was a valid production at this index in the parse table, then retrieve the right-hand side of the production predicted by the parse table
    			vector<int> predicted_production = right_hand_sides[parse_tab[abs(expected_symbol) - 1][input_tok.num - 1] - 1];


    			// Print out the non-terminal that is being reduced for the user's information
    			cout << "Predicted: " << nonterminal_names[expected_symbol] << " -> ";
				parse_stack.pop(); // Remove the element to be reduced from the parse stack

				int i = 0; // Loop counter
				// As long as a zero has not been found in the right-hand side array (zero acts as a sentinel value)
				while(predicted_production[i] != 0) {
					// Push each symbol in the right hand side of the production onto the parse stack
					parse_stack.push(predicted_production[i]);
					++i; // Increment the loop counter
				}
				// Handling the output of what symbols were predicted. Since the right-hand side array contains a reversed order to support stack pushes, the first element to print is
				// the symbol represented at the end of the right-hand side array rather than the beginning, therefore the loop decrements rather than increments
				for(i = i-1; i >= 0; i--) {
					if(predicted_production[i] < 0)
						cout << terminal_names[abs(predicted_production[i])] << " ";
					else
						cout << nonterminal_names[abs(predicted_production[i])] << " ";
				}
				i = 0; // Reset the loop counter
				cout << "\n"; // Finalize the output to the user with a newline

			}
    	} else
    		cout << "Undefined symbol at the top of parse stack\n"; 
    		// The expected symbol at the top of the parse stack is equal to 0, which is undefined and not used


    } while (parse_stack.top() != -1);
    // Loop for as long as the top of the parse stack does not represent the end of a parse (-1)
    cout << "Out of the parse loop\n";
}

// Meta-function for generating data structures for parsing. Must be called before parse()
void generate(char* inputName) {

	// Fluse stdout just to be safe
	fflush(stdout);

	count_input(inputName); // Initialize max_terminal and num_nonterminals

	vector<vector<string> > nonterm_and_term_names = init_names(inputName); // Read input to initialize nonterm/terminal names
	terminal_names = nonterm_and_term_names[0]; // Returns a two-dimensional vector, assign each vector to the results
	nonterminal_names = nonterm_and_term_names[1];

	vector<bool> generates_epsilon(num_nonterminals); // Initialize generates_epsilon to the number of non-terminals in the grammar
	

	full_productions = init_productions(inputName, generates_epsilon); // Find and assign the full productions array

	

	first_sets = init_first(generates_epsilon); // Create/return first sets

	

	follow_sets = init_follow(generates_epsilon, first_sets); // Create/return follow sets


	
	predict_sets = init_predict(generates_epsilon, first_sets, follow_sets); // Create/return predict sets for each production

	parse_tab.resize(nonterminal_names.size(), vector<int>(max_terminal)); // Resize the parse table to accomodate the number of elements necessary

	// For each element in the parse table, initialize to 0 for later parsing purposes
	for(int i = 0; i < nonterminal_names.size(); i++) {
		for(int j = 0; j < max_terminal; j++) {
			parse_tab[i][j] = 0;
		}
	}

	// For each element in the predict sets, find the apprporiate location in the parse table, and place the correct terminals
	for(int i = 0; i < predict_sets.size(); i++) {
		for(int j = 0; j < predict_sets[i].size(); j++) {
			if(parse_tab[full_productions[i][0] - 1][predict_sets[i][j] - 1]) {
				cout << "Attempting to access a location in the parse table that has already been initializd indicates an input grammar that is not LL(1). Exiting the program";
				exit(0);
			}
			parse_tab[full_productions[i][0] - 1][predict_sets[i][j] - 1] = i + 1;
		}
	}

	// Resize the rright_hand_sides vector to hold as many locations as there are productions. Inner vectors initializes to 0 but are resized with calls to push_back
	right_hand_sides.resize(full_productions.size(), vector<int>(0));
	for(int i = 0; i < full_productions.size(); i++) {
		for(int j = full_productions[i].size() - 1; j > 0; j--) {
			if(full_productions[i][j] != 0) right_hand_sides[i].push_back(full_productions[i][j]); // Elements are iterated in reverse order to facilitate stack implementation
		}
		right_hand_sides[i].push_back(0); // Terminate every vector with a 0 for checks in the parsing phase
	}
}

// Initializes the predict sets for each production in the grammar. Used to generate the parse table and depends upon the creation of both the first and follow
// sets before being executed
vector<vector<int> > init_predict(vector<bool> local_generates_epsilon, vector<vector<int> > local_first_sets, vector<vector<int> > local_follow_sets) {
	
	vector<vector<int> > temp_predict_sets(num_productions, vector<int>(0)); // A temporary vector that will be returned.

	// For each symbol in each production in the grammar
	for(int i = 0; i < full_productions.size(); i++) {
		for(int j = 1; j < full_productions[i].size(); j++) {

			// If this element is 0, then we have reached the end of the production and the follow set of the the left-hand side is in the predict set
			// This means that each of the previous elements could generate epsilon, otherwise the inner loop would have exited
			if(full_productions[i][j] == 0) {

				temp_predict_sets[i] = push_unique_elements(temp_predict_sets[i], local_follow_sets[full_productions[i][0]]); // Only push unique elements into the vector
				break; // Continue to the next production - this production has been fully explored

			} else if(full_productions[i][j] < 0) { // If we have gotte to this symbol and this symbol represents a terminal

				// Then this element must be in the predict set
				temp_predict_sets[i].push_back(abs(full_productions[i][j]));
				break; // Terminals cannot generate epsilon, therefore we are done exploring this line

			} else { // Otherwise this element is a non-terminal

				// Since we have iterated to this nonterminal, then the first of this nonterminal is in the predict set
				temp_predict_sets[i] = push_unique_elements(temp_predict_sets[i], local_first_sets[full_productions[i][j]]); // Only push unique elements

				// If this element generates epsilon
				if(local_generates_epsilon[full_productions[i][j] - 1]) {

					// And if the next element is a 0 (representing the end of the production)
					if(full_productions[i][j + 1] == 0) {

						// Then the folow of the left hand side of this production is also in the predict set
						temp_predict_sets[i] = push_unique_elements(temp_predict_sets[i], local_follow_sets[full_productions[i][0]]);
						break; // No more information can be gathered from this production as the next element will be a 0
					}
					continue; // Somewhat unnecessary continue
				} else break; // If this non-terminal did not generate epsilon, then this is the end of the exploration and the predict set has been found
			}
		}
	}

	return temp_predict_sets; // Return the temporary predict

}

// Initializes and returns a two-dimensional vector containing the follow sets of each non-terminal in the grammar. Indexes of
// each vector in the first dimension similarly correspond with nonterminal indices in nonterminal_names
vector<vector<int> > init_follow(vector<bool> local_generates_epsilon, vector<vector<int> > local_first_sets ) {

	vector<vector<int> > temp_follow(num_nonterminals + 1, vector<int>(0)); // A temporary vector that will be returned.
	int count = 0;

	// Pass over the input twenty times to ensure follow sets are created accurately
	while(count < 20) {

		// For each element in each production
		for(int i = 0; i < full_productions.size(); i++) {
			for(int j = 1; j < full_productions[i].size(); j++) {

				// If the production is a terminal, then this element does not have a follow set, and we can continue
				if(full_productions[i][j] < 0) continue;
				else if(full_productions[i][j] > 0) { // Otherwise, if this element is a nonterminal, then it's follow set can be updated

					// For each element left in the production following this current element
					for(int k = j + 1; k < full_productions[i].size(); k++) {

						// If the next symbol is 0 (end of the production), and the follow set of the left hand side of this production has something inside of it
						if(full_productions[i][k] == 0 && temp_follow[full_productions[i][0]].size() != 0) {

							// Then add the follow of the left hand side to the follow of this element
							temp_follow[full_productions[i][j]] = push_unique_elements(temp_follow[full_productions[i][j]], temp_follow[full_productions[i][0]]);

						// Otherwise, if there is nothing in the follow of the following symbol, then continue to the next element
						} else if(temp_follow[full_productions[i][k]].size() == 0) break;

						// If neither of the above cases are true, and the following symbol is a terminal
						else if(full_productions[i][k] < 0) {

							// And if the element is not already in the follow of the [i][j] symbol
							if(!find_symbol_in_array(temp_follow[full_productions[i][j]], abs(full_productions[i][k]))) {
								// Then add this terminal to the follow of the [i][j]th nonterminal
								temp_follow[full_productions[i][j]].push_back(abs(full_productions[i][k]));
							}
							break; // In this case, then this terminal can not generate epsilon, and there is no more information to be gathered
						}
						// If the next symbol is a nonterminal, then add it's first to the follow of the [i][j]th element
						else if(full_productions[i][k] > 0) {
							temp_follow[full_productions[i][j]] = push_unique_elements(temp_follow[full_productions[i][j]], local_first_sets[full_productions[i][k]]);

							// If this nonterminal also generates epsilon, then continue to explore the elements following the [i][j]th element
							if(local_generates_epsilon[full_productions[i][k] - 1]) continue;
							else break; // Otherwise if this nonterminal does not generate epsilon, then no more inforrmation can be gathered
						}
					}
				} else break; // A catch all for cases that do not meet any of the above criteria

			}
		}
			++count; // Increment the loop counter
	}

		return temp_follow; // Return to the temporary vector
}

// Pushes a vector of integers into a destination vector and returns a vector containing all elements of both arrays without duplicates
vector<int> push_unique_elements(vector<int> destination, vector<int> source) {

	// For each element in the source vector
	for(int i = 0; i < source.size(); i++) {
		// If this symbol cannot be found in the array, then add it to the destination vector
		if(!find_symbol_in_array(destination, abs(source[i]))) {
			destination.push_back(source[i]);
		}
	}
	return destination; // Returm the modified desination vector

}

// Initializes and returns two-dimensional vector containing the first sets of each non-terminal in the grammar. Indexes of each
// vector in the first dimension correspond with nonterminal indices in nonterminal_names
vector<vector<int> > init_first(vector<bool> local_generates_epsilon) {

	vector<vector<int> > temp_first(num_nonterminals + 1, vector<int>(0)); // Temporary vector for hold first sets
	bool complete = false; // Loop control

	// As long as information can be gathered from another loop
	while(!complete) {

		// Assume that this iteration is the last one
		complete = true;

		// For each element in everry production
		for(int i = 0; i < full_productions.size(); i++) {
			for(int j = 1; j < full_productions[i].size(); j++) {

				// If this is the end of the production, then break
				if(full_productions[i][j] == 0) {
					break;
				// Otherwise, if this symbol is a nonterminal
				} else if(full_productions[i][j] < 0) {

					// And if this element has not already been added to the left hand side's first set
					if(!find_symbol_in_array(temp_first[full_productions[i][0]], full_productions[i][j])) {

						// Then add it to the left hand side's first set
						temp_first[full_productions[i][0]].push_back(abs(full_productions[i][j]));
						complete = false; // A modification was made, so another pass might be necessary
						break; // This element was a terminal and cannot generate epsilon, therefore break

					} else break; // If the element was found in the first set already, then just continue to the next production

				// If this element is a nonterminal and it's first set is not already in the first set of the left hand side	
				} else if(full_productions[i][j] > 0 && temp_first[full_productions[i][j]].size() > 0) {

					vector<int> current_elements_first = temp_first[full_productions[i][j]]; // Probably unnecessary

					// This section was written early and might be replacable with a call to push_unique_elements. It accomplishes the same thing
					for(int k = 0; k < current_elements_first.size(); k++) {
						if(!find_symbol_in_array(temp_first[full_productions[i][0]], current_elements_first[k])) {
							temp_first[full_productions[i][0]].push_back(current_elements_first[k]);
							complete = false;
						}
					}
				}

				// If this element does not generate epsilon, then continue to the next production
				if(!local_generates_epsilon[abs(full_productions[i][j]) - 1]) {
					break;
				}
				 
			}
		}
	}

	return temp_first; // Return the temporary vectorr
			
}

// Finds if a give symbol is inside of a given array. If the element is found in the given array, then the function returns true, otherwise it returns false
bool find_symbol_in_array(vector<int> find_in, int element) {

	// If there is nothing to look in, then trivial case
	if(find_in.size() == 0) return false;

	// For each element in the vector, if the element is present return true, otherwise false
	for(int i = 0; i < find_in.size(); i++) {
		if(find_in[i] == abs(element)) return true;
	}
	return false;
}


// Initializes and returns a two-dimension vector containing numeric representations of each terminal/nonterminal as dictated
// by the index in nonterminal_names and terminal_names respectively. Terminals are stored as negative numbers to facilitate checking
// Also creates the generates_epsilon vector as an intended side-effect
vector<vector<int> > init_productions(char* inputFile, vector<bool> &func_generates_epsilon) {

	vector<vector<int> > temp_productions(num_productions, vector<int> (max_terminal + num_nonterminals)); // Temporary storage

	int symbol_count = 0; // The number of symbols that have been currently explored on the line
	int line_count = 0; // The current line (relative to the start of the proudction section of the grammar)
	string current_symbol; // A string holding the current symbol that is being explored

	input.open(inputFile); // Open the input file

	// Skip to the productions
	while(getline(input, line)) {
		if(line.compare("") == 0) break;
	}

	// As long as there is a line to get from the input file
	while(getline(input, line)) {

		// Reset symbol count to ensure that this is the first symbol of the production
		symbol_count = 0;
		
		stringstream ss(line); // A string stream for tokenizing the current line

		// First get the production's left hand side
		getline(ss, current_symbol, ' ');

		// Save the left side separately
		string left_side = current_symbol;
		temp_productions[line_count][symbol_count] = find_in_symbol_names(current_symbol); // Place the left hand side into productions
		++symbol_count; // Increment the symbol counter
		getline(ss, current_symbol, ' '); // Skip over the meta-symbol in the middle of the production

		// If this is the end of the string stream, then this production generates epsilon
		if(ss.eof())  {
			func_generates_epsilon[find_in_symbol_names(left_side) - 1] = true; // Assign the approprriate index of generates epsilon to true
		}

		// For as long as there are symbols on the current line
		while(getline(ss, current_symbol, ' ')) {

			temp_productions[line_count][symbol_count] = find_in_symbol_names(current_symbol); // Add this number representing this symbol to the productions vector
			++symbol_count; // On to the next symbol in the string
		}

		// Once the line has been fully input, on to the next line
		++line_count;

	}

	// Close the file and return the productions vectorr
	input.close();

	return temp_productions;
		
}

// Finds a symbol (nonterminal or terminal) and returns the index of the queried symbol in nonterminal or terminal names. Negative values represent terminals while
// positive values represent nonterminals. If not found, returns an arbitrary 999;
int find_in_symbol_names(string symbol_to_find) {

	// Check in the terminal names
	for(int i = 0; i < terminal_names.size(); i++) {
		if(terminal_names[i].compare(symbol_to_find) == 0)
			return i * -1;
	}

	// Check in the nonterminal names
	for(int i = 0; i < nonterminal_names.size(); i++) {
		if(nonterminal_names[i].compare(symbol_to_find) == 0)
			return i;
	}

	// If nothing has been found, then return something arbitrary
	return 999;

}

// Initializes vectors containing the names of each terminal/nonterminal. For termainals, strings are placed at the index
// indicated by the number provided in the grammar file, whereas nonterminals are placed sequentially starting at 1;
vector<vector<string> > init_names(char* inputFile) {

	vector<string> temp_terminals(max_terminal + 1); // Terminal names
	vector<string> temp_nonterminals(num_nonterminals + 1); // Nonterminal names

	// Aggregate vector for returning both arrays at once
	vector< vector<string> > temp_names(2);
	temp_names[0] = temp_terminals;
	temp_names[1] = temp_nonterminals;

	input.open(inputFile); // Open the file

	string current_terminal_name; // The current terminal's name and index are stored separately
	string current_terminal_index;

	// As long as there is a line to get
	while(getline(input, line)) {

		// If this line contains nothing, this signals the beginning of productions section of the grammar
		if(line.compare("") == 0) break;

		stringstream ss(line); // Create a stringstream for parsing this line individually
		getline(ss, current_terminal_name, ' '); // The terminal name is given first
		getline(ss, current_terminal_index, ' '); // The index of that terminal is given second

		// Place that terminal name at the appropriate index in the terminal names vectorr
		temp_names[0][atoi(current_terminal_index.c_str())] = current_terminal_name;
	}

	int current_nonterminal_index = 0; // Current nonterminal index is used to prrevent restoring the same nonterminal for redundant prroductions
	string current_nonterminal_name; // Current name of this nonterminal
	string last_unique_nonterminal(""); // The last unique nonterminal to be gotten (prevents redundancy)

	// For as long as there are lines still in the input file
	while(getline(input, line)) {

		stringstream ss(line); // Create a stringstream for parsing this individual line

		getline(ss, current_nonterminal_name, ' '); // The currernt nonterminal name is on the left hand side of the production

		// If that name is different from the last unique nonterminal that we have seen
		if(current_nonterminal_name.compare(last_unique_nonterminal) != 0) {
			temp_names[1][current_nonterminal_index + 1] = current_nonterminal_name; // Then place this nonterminal into the nonterminal names vector
			++current_nonterminal_index; // Current nonterminal index is incremeneted to provide for consistent indexing of sequential nonterrminals
			last_unique_nonterminal = current_nonterminal_name; // Set the last unique nonterminal as the current nonterminal
		}
	}
 
	input.close(); // Close the input file

	return temp_names; // Return the temporary arrays containing names
}

// Initializes max_terminal and num_productions with an initial scan of the grammar file
void count_input(char* inputFile) {

	input.open(inputFile); // Open the input file

	string current_max_terminals; // The currrent max number of terminals (the highest number that we have seen so far)

	// For as long as there are lines in the file and it is not the beginning of the productions section
	while(getline(input, line)){
		if(line.compare("") == 0) break;

		stringstream ss(line); // Create a stringstream to parse each individual line

		getline(ss, current_max_terminals, ' ');
		getline(ss, current_max_terminals, ' '); // Get to the numeric representating of the terminals and store it
		max_terminal = atoi(current_max_terminals.c_str());

	}

	string last_unique_nonterminal(""); // The last unique nonterminal that we have seen so far
	string this_nonterminal; // The nonterminal currently being examined

	// For as long as there are still lines in the input file
	while(getline(input, line)) {
		
		stringstream ss(line); // Create a stringstream on this individual line

		getline(ss, this_nonterminal, ' '); // Get the nonterminal's name

		// If this is a new nonterminal, then increment the number of nonterminals and set the last unique nonterminal
		if(this_nonterminal.compare(last_unique_nonterminal) != 0) {
			++num_nonterminals;
			last_unique_nonterminal = this_nonterminal;
		}

		++num_productions; // increment the total number of productions as well even if this is the same nonterminal
	}

	input.close(); // Close the input file
}