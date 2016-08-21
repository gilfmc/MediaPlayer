#include "ImpreciseDate.h"

ImpreciseDate::ImpreciseDate(int year, int month, int dayOfMonth) : _year(year), _month(month), _dayOfMonth(dayOfMonth), _hasYear(year != -1) { }

ImpreciseDate::ImpreciseDate(const ImpreciseDate & date) : _year(date._year), _month(date._month), _dayOfMonth(date._dayOfMonth), _hasYear(date._hasYear) { }

int ImpreciseDate::dayOfMonth() const
{
	return _dayOfMonth;
}

int ImpreciseDate::month() const
{
	return _month;
}

int ImpreciseDate::year() const
{
	return _year;
}

bool ImpreciseDate::hasDayOfMonth() const
{
	return _dayOfMonth > 0;
}

bool ImpreciseDate::hasMonth() const
{
	return _month > 0;
}

bool ImpreciseDate::hasYear() const
{
	return _hasYear;
}

bool ImpreciseDate::operator<(const ImpreciseDate & other) const
{
	if(_year < other._year) return true;
	else if(_year == other._year) {
		if(_month < other._month) return true;
		else if(_month == other._month) {
			if(_dayOfMonth < other._dayOfMonth) return true;
			else return false;
		}
		
		return false;
	}
	
	return false;
}

bool ImpreciseDate::operator>(const ImpreciseDate & other) const
{
	if(_year > other._year) return true;
	else if(_year == other._year) {
		if(_month > other._month) return true;
		else if(_month == other._month) {
			if(_dayOfMonth > other._dayOfMonth) return true;
			else return false;
		}
		
		return false;
	}
	
	return false;
}

