#include<iostream>
#include<ctime>
#include<vector>
#include<string>
#include<algorithm>
#include<cstdio>
#include<cstdlib>
#include<fstream>
#include<map>
#include<cstring>
#include<cmath>
#include<sstream>

#define FAIL false
#define SUCCESS true

using namespace std;

typedef map<pair<string, int>, int> ProductTimeCount;

const string month[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"};

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

class VoteRatio {
public:
	int helpful;
	int all;
	VoteRatio() {
		helpful =0;
		all = 0;
	}
};

vector<Review> reviews, filter_reviews;
vector<Product> products;
// Number of fine food items purchased from Amazon is different years and months of years
// Same months over different years are accumulated.
int overall_count_month[12];
int overall_count_year[2020];

// item_count_per_month has year*100+month as the time window.
// map from (product_id, time range) -> count
map<pair<string, int>, int> item_count_per_year, item_count_per_month;

// Map from user_id to the number of reviews they have
map<string, int> count_review_per_user;

// Map from the length of video to the average of helpfulness for reviews which have video
map<int, double> length_to_rating;
// Assuming 30 min is the longest video
vector<double> video_votes[60*10+10], helpful_video_votes[60*10+10];

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

void MyFilter(string key, string value) {
	filter_reviews.clear();
	for ( int i = 0; i < (int)reviews.size(); i++){
		if (key == "productId") {
			if(reviews[i].product_id == value) {
				filter_reviews.push_back(reviews[i]);
			}
		}
	}
	reviews = filter_reviews;
}

// Number of reviews written in different months.
// The number for a month is accumulated over all the years.
void CountMonthlyAccumulatedReviews() {
	for (int i = 0; i < (int)reviews.size(); i++) {
		overall_count_month[reviews[i].time.month]++;
	}
	ofstream overall_outputs_monthly_accumulated_out("../Output_FineFoods/overall_monthly_accumulated.txt");
	for (int i = 0; i < 12; i++) {
		overall_outputs_monthly_accumulated_out << month[i] << " " << overall_count_month[i] << endl;
	}
}

// Number of reviews written in different years.
void CountYearlyReviews() {
	for (int i = 0; i < (int)reviews.size(); i++) {
		overall_count_year[reviews[i].time.year]++;
	}
	ofstream overall_outputs_yearly_out("../Output_FineFoods/overall_yearly.txt");
	for (int i = 1998; i < 2015; i++) {
		overall_outputs_yearly_out << i << " " << overall_count_year[i] << endl;
	}
}

// Number of reviews in different months for a particular item.
// Separate years are not accumulated.
void PerItemPerMonth() {
	for (int i = 0; i < (int)reviews.size(); i++) {
		pair<string, int> product_id_time_range;
		product_id_time_range = make_pair(reviews[i].product_id, reviews[i].time.year);
		item_count_per_year[product_id_time_range]++;
	}
	ProductTimeCount::iterator before;
	ofstream fout;
	for ( ProductTimeCount :: iterator current = item_count_per_month.begin();
			current != item_count_per_month.end(); current++){
		if (current == item_count_per_year.begin() ||
				before->first.first != current->first.first) {
			fout.close();
			fout.open(("../Output_FineFoods/PerItem/" + current->first.first + "_monthly.txt").c_str(),std::ofstream::out);
			//	fout << current ->first.first << endl;
		}
		fout << month[current->first.second%100] << "/" <<
				current->first.second/100 << " " << current->second <<endl;
		before = current;
	}
}

// Number of reviews in different years for a particular item.
void PerItemPerYear() {
	for (int i = 0; i < (int)reviews.size(); i++) {
		pair<string, int> product_id_time_range;
		product_id_time_range = make_pair(reviews[i].product_id,
				reviews[i].time.year*100 + reviews[i].time.month);
		item_count_per_month[product_id_time_range]++;
	}
	ProductTimeCount::iterator before;
	ofstream fout;
	for ( ProductTimeCount :: iterator current = item_count_per_year.begin();
			current != item_count_per_year.end(); current++){
		if (current == item_count_per_year.begin() ||
				before->first.first != current->first.first) {
			fout.close();
			fout.open(("../Output_FineFoods/PerItem/" + current->first.first + "_yearly.txt").c_str(),std::ofstream::out);
			//	fout << current ->first.first << endl;
		}
		fout << current->first.second << " " << current->second <<endl;
		before = current;
	}

}

// Outputs the top size_of_list products that have been reviewed more than others.
void TopProducts(int size_of_list) {
	for (int i = 0; i <(int) reviews.size(); i++) {
		product_count[reviews[i].product_id]++;
	}
	Product product;
	ofstream top_products_out("../Output_FineFoods/top_products.txt");
	for (map<string, int>::iterator it = product_count.begin(); it!=product_count.end(); it++) {
		product.product_id = it->first;
		product.count = it->second;
		products.push_back(product);
	}
	sort (products.begin(), products.end());
	for (int i = 0 ; i < size_of_list; i++) {
		top_products_out << products[i].product_id << " " << products[i].count << endl;
	}

	// Number of reviewers with certain number of reviews
	for (int i = 0; i < (int)reviews.size(); i++) {
		count_review_per_user[reviews[i].user_id]++;
	}
}

double FindAverage(vector<double> *data) {
	if (data->size() == 0) {
		return 0;
	}
	double average = 0;
	for (int i = 0; i < (int)data->size(); i++) {
		average += (*data)[i];
	}
	return average / data->size();
}

double FindError(vector<double> *data, double average) {
	if (data->size() == 0) {
		return 0;
	}
	double error = 0;
	for (int i = 0; i < (int)data->size(); i++) {
		error += ((*data)[i]-average) * ((*data)[i]-average);
	}

	error /= data->size();
	return sqrt(error);
}

// All the data that is analyzed have at least one vote.
void ReviewsWithVideo() {
	double video_ratio = 0, data_ratio = 0;
	double video_average, data_average;
	double video_error = 0, data_error = 0;
	int length_of_video, minute, second;
	const int time_bucket = 60;
	int number_of_videos = 0;

	for (int i = 0; i < (int)reviews.size(); i++) {
		stringstream ss(reviews[i].helpfulness);
		int numerator, denominator;
		char ch;
		ss>> numerator >> ch >> denominator;
		if (denominator == 0) {
			continue;
		}
		data_ratio += (double)numerator / denominator;
		if (reviews[i].text.find("Length::") != string::npos) {
			video_ratio += (double)numerator / denominator;
			number_of_videos++;
			int position = reviews[i].text.find("Length::");
			string has_the_length = reviews[i].text.substr(position+8);
			stringstream get_length(has_the_length);
			get_length >> minute >> ch >> second;
			length_of_video = minute * 60 + second;
			video_votes[length_of_video / time_bucket].push_back((double)numerator / denominator);
		}
	}
	data_average = data_ratio / reviews.size();
	video_average = video_ratio / number_of_videos;
	for (int i = 0; i < (int)reviews.size(); i++) {
		stringstream ss(reviews[i].helpfulness);
		int numerator, denominator;
		char ch;
		ss>> numerator >> ch >> denominator;
		if (denominator == 0) {
			continue;
		}
		data_error += ((double)numerator / denominator - data_average) * ((double)numerator / denominator - data_average);
		if (reviews[i].text.find("Length::") != string::npos) {
			video_error += ((double)numerator / denominator - video_average) * ((double)numerator / denominator - video_average);
		}
	}
	data_error /= reviews.size();
	video_error /= number_of_videos;

	ofstream video_correlation_to_helpfulness("../Output_FineFoods/video_correlation_to_helpfulness.out");
	video_correlation_to_helpfulness << "Number of videos " << number_of_videos << endl;
	video_correlation_to_helpfulness << "Average of video and data: " << video_average << " " << data_average <<endl;
	video_correlation_to_helpfulness << "Error of video and data: " << video_error << " " << data_error <<endl;

	ofstream video_length_correlation_to_helpfulness(
			"../Output_FineFoods/video_correlation_to_helpfulness_has_error.txt");
	for (int i = 0; i < 600/time_bucket ; i++) {
		double average, error;
		average = FindAverage(&video_votes[i]);
		error = FindError(&video_votes[i], average);
		video_length_correlation_to_helpfulness << "[" << i*60 << "," << i*60+60 << "] " << average << " " << error <<endl;
	}
}

// Number of reviews written in different months.
// The number for a month is accumulated over all the years.
void StarAveragePerMonth() {
	vector<double> star_rating[12];
	double average;
	double error;
	for (int i = 0; i < (int)reviews.size(); i++) {
		stringstream ss(reviews[i].score);
		double score;
		ss >> score;
		star_rating[reviews[i].time.month].push_back(score);
	}
	ofstream overall_outputs_monthly_accumulated_out_star_rating("../Output_FineFoods/overall_monthly_accumulated_star_rating_has_error.txt");
	for (int i = 0; i < 12; i++) {
		average = FindAverage(&star_rating[i]);
		error = FindError(&star_rating[i], average);
		overall_outputs_monthly_accumulated_out_star_rating << month[i] << " " << average << " " << error << endl;
	}
}

// Number of reviews written in different years.
void StarAveragePerYear() {
	vector<double> star_rating[30];
	double average;
	double error;
	for (int i = 0; i < (int)reviews.size(); i++) {
		stringstream ss(reviews[i].score);
		double score;
		ss >> score;
		star_rating[reviews[i].time.year-1998].push_back(score);
	}
	ofstream overall_outputs_yearly_out_star_rating("../Output_FineFoods/overall_yearly_star_rating_has_error.txt");
	for (int i = 1998; i < 2015; i++) {
		average = FindAverage(&star_rating[i - 1998]);
		error = FindError(&star_rating[i - 1998], average);
		overall_outputs_yearly_out_star_rating << i << " " << average  << " " << error << endl;
	}
}

void StarAveragePerTimeInTheDay() {
	vector<double> star_rating[30];
	double average;
	double error;
	for (int i = 0; i < (int)reviews.size(); i++) {
		stringstream ss(reviews[i].score);
		double score;
		ss >> score;
		star_rating[reviews[i].time.hour].push_back(score);
	}
	ofstream overall_outputs_hourly_accumulated_out_star_rating("../Output_FineFoods/overall_hourly_accumulated_star_rating_has_error.txt");
	for (int i = 0; i < 24; i++) {
		average = FindAverage(&star_rating[i]);
		error = FindError(&star_rating[i], average);
		overall_outputs_hourly_accumulated_out_star_rating << i << " " << average  << " " << error << endl;
	}
}

int main() {
	// Read input.
	while (true) {
		if (!ReadOneReview()) {
			break;
		}
	}
	MyFilter("productId", "B0032JKWGI");
	/*
	CountMonthlyAccumulatedReviews();
	CountYearlyReviews();

	PerItemPerMonth();
	PerItemPerYear();

	// Top products.
	int size_of_list = 10;
	TopProducts(size_of_list);

	// Video Average vs All average.
	ReviewsWithVideo();
	*/
	StarAveragePerMonth();
	StarAveragePerYear();
	// Time in Day is useless! The timestamp is on a daily basis
	//StarAveragePerTimeInTheDay();
	return 0;
}

