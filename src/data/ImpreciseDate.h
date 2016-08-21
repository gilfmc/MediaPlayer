#ifndef IMPRECISEDATE_H
#define IMPRECISEDATE_H

class ImpreciseDate
{
private:
    int _year;
    char _month;
    char _dayOfMonth;
    bool _hasYear;
	
public:
	ImpreciseDate(int year = -1, int month = -1, int dayOfMonth = -1);
	ImpreciseDate(const ImpreciseDate & date);
	
	bool hasYear() const;
	bool hasMonth() const;
	bool hasDayOfMonth() const;
	
	int year() const;
	int month() const;
	int dayOfMonth() const;
	
	bool operator<(const ImpreciseDate &) const;
	bool operator>(const ImpreciseDate &) const;
};

#endif // IMPRECISEDATE_H
