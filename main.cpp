#include<iostream>
#include<ctime>
#include<vector>
#include<string>
#include<algorithm>
#include<cstdio>
#include<cstdlib>
#include<fstream>
#include<map>

#define FAIL false
#define SUCCESS true

using namespace std;

typedef map<pair<string, int>, int> ProductTimeCount;

const string month[] = {"JAN", "FEB", "MAR", "APR", "MAY", "JUNE", "JULY", "AUG", "Sep", "Oct", "Nov", "Dec"};

map<string, int> product_count;

class MyTime { 
  public:
	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int weekday;

	MyTime(){}

	MyTime(struct tm* time){
		year = time->tm_year+1900;
		month = time->tm_mon;
		day = time->tm_mday;
		hour = time->tm_hour;
		minute = time->tm_min;
		second = time->tm_sec;
		weekday = time->tm_wday;
	}
};

class Review {
	public:
		string product_id;
		string user_id;
		string profile_name;
		string helpfulness;
		string score;
		MyTime time;
		string summary;
		string text;
		
		void print() {
			cout<< "ProductId: " << product_id << endl;
			cout<< "UserId: " << user_id << endl;
			cout<< "ProfileName: " << profile_name << endl;
			cout<< "Helpfulness: " << helpfulness << endl;
			cout<< "Score: " << score << endl;
			cout<< "Time: " << time.month << "/" << time.day << "/" << time.year<< "   ";
			cout<< time.hour << ":" << time.minute << ":" << time.second <<endl;
			cout<< "Summary: " << summary << endl;
			cout<< "Text: " << text << endl;
		}
};

class Product { 
	public:
		string product_id;
		int count;
	bool operator < (const Product &other) const { 
		return count > other.count;
	}
};

vector<Review> reviews; 
vector<Product> products;
// Number of fine food items purchased from Amazon is different years and months of years
// Same months over different years are accumulated.
int overall_count_month[12];
int overall_count_year[2020];

// item_count_per_month has year*100+month as the time window.
// map from (product_id, time range) -> count
map<pair<string, int>, int> item_count_per_year, item_count_per_month;

string GetField(string raw_input) {
	int delimeter = raw_input.find(":");
	if (delimeter == std::string::npos) {
		return "THIS INPUT IS TRASH";
	}
	return raw_input.substr(delimeter+2);
}


bool ReadOneReview() {
	string raw_input;
	Review review;
	if (getline(cin, raw_input)) {
		review.product_id = GetField(raw_input);
		getline(cin, raw_input);
		review.user_id = GetField(raw_input);
		getline(cin, raw_input);
		review.profile_name = GetField(raw_input);
		do {
			getline(cin, raw_input);
			review.helpfulness = GetField(raw_input);
		} while (review.helpfulness == "THIS INPUT IS TRASH");
		getline(cin, raw_input);
		review.score = GetField(raw_input);
		getline(cin, raw_input);
		int time_int = atoi((GetField(raw_input)).c_str());
		time_t review_time(time_int);
		review.time = MyTime(localtime(&review_time));
		getline(cin, raw_input);
		review.summary = GetField(raw_input);
		getline(cin, raw_input);
		review.text = GetField(raw_input);
		getline(cin, raw_input);
		reviews.push_back(review);
		return SUCCESS;
	}
	return FAIL;
}


int main() {

	// Read input.
	while (true) {
		if (!ReadOneReview()) {
			break;
		}
	}

	// Month wise and yearly.
	for (int i = 0; i < reviews.size(); i++) {
		overall_count_month[reviews[i].time.month]++;
		overall_count_year[reviews[i].time.year]++;
		product_count[reviews[i].product_id]++;
		
		pair<string, int> product_id_time_range;
		product_id_time_range = make_pair(reviews[i].product_id, reviews[i].time.year);
		item_count_per_year[product_id_time_range]++;
	
		product_id_time_range = make_pair(reviews[i].product_id,
						  reviews[i].time.year*100 + reviews[i].time.month);
		item_count_per_month[product_id_time_range]++;
	}
	ofstream overall_outputs_monthly_accumulated_out("Output/overall_monthly_accumulated.txt");
	ofstream overall_outputs_yearly_out("Output/overall_yearly.txt");
	for (int i = 0; i < 12; i++) { 
		overall_outputs_monthly_accumulated_out << month[i] << " " << overall_count_month[i] << endl;
	}
	for (int i = 1998; i < 2015; i++) { 
		overall_outputs_yearly_out << i << " " << overall_count_year[i] << endl;
	}

	// Per item histogram data.
	ProductTimeCount::iterator before;
	ofstream fout;
	for ( ProductTimeCount :: iterator current = item_count_per_year.begin();
		current != item_count_per_year.end(); current++){
		if (current == item_count_per_year.begin() || 
		    before->first.first != current->first.first) {
			fout.close();
			fout.open(("Output/PerItem/" + current->first.first + "_yearly.txt").c_str(),std::ofstream::out);
			fout << current ->first.first << endl;
		}
		fout << current->first.second << " " << current->second <<endl;
		before = current;
	}
	
	for ( ProductTimeCount :: iterator current = item_count_per_month.begin();
		current != item_count_per_month.end(); current++){
		if (current == item_count_per_year.begin() ||
		    before->first.first != current->first.first) {
			fout.close();
			fout.open(("Output/PerItem/" + current->first.first + "_monthly.txt").c_str(),std::ofstream::out);
			fout << current ->first.first << endl;
		}
		fout << month[current->first.second%100] << "/" <<
			current->first.second/100 << " " << current->second <<endl;
		before = current;
	}

	// Top products.
	Product product;
	ofstream top_products_out("Output/top_products.txt");
	for (map<string, int>::iterator it = product_count.begin(); it!=product_count.end(); it++) {
		product.product_id = it->first;
		product.count = it->second;
		products.push_back(product);
	}
	sort (products.begin(), products.end());
	for (int i = 0 ; i < 10; i++) {
		top_products_out << products[i].product_id << " " << products[i].count << endl;
	}

	return 0;
}

