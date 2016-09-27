//
// Copyright (C) 2015-2016 University of Amsterdam
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef SPSSRECINTER_H
#define SPSSRECINTER_H

#include "missingvaluechecker.h"
#include "measures.h"
#include "spssformattype.h"
#include "numericconverter.h"

#include "measures.h"
#include "spssformattype.h"
#include "../convertedstringcontainer.h"
#include <column.h>

#include <vector>
#include <string>
#include <set>
#include <map>
#include <qdatetime.h>

/*
 * Macros to build attriutes of a class.
 */
#define _ATT_VALUE(type, name)		\
private:							\
	type _##name;					\

#define READ_ATTR(type, name)		\
	_ATT_VALUE(type, name)			\
public:								\
	const type & name() const		\
		{ return _##name; }			\

#define WRITE_ATTR(type, name)		\
	_ATT_VALUE(type, name)			\
public:								\
	void name(const type &value)	\
		{ _##name = value; }		\

#define RW_ATTR(type, name)			\
	READ_ATTR(type, name)			\
public:								\
	void name(const type &value)	\
		{ _##name = value; }		\


namespace spss
{

/**
 * A type that holds the data in an intermate format
 */
class SPSSColumn : ConvertedStringContainer
{
public:
	typedef std::map<SpssDataCell, std::string>		LabelByValueDict;
	typedef std::pair<SpssDataCell, std::string>	LabelByValueDictEntry;


	/**
	 * @brief SPSSColumn Ctor
	 * @param name SPSS column name (short form)
	 * @param label Column label.
	 * @param stringLen String length or zero.
	 * @param formattype The format Type from the variable record.
	 * @param missingChecker Check for missing value with this.
	 */
	SPSSColumn(const std::string &name, const std::string &label, long stringLen, FormatTypes formattype, const spss::MissingValueChecker &missingChecker);

	~SPSSColumn();

	/*
	 * Attributes for the column.
	 */
	WRITE_ATTR(std::string, spssLongColName) // The name as shown to the user.
	WRITE_ATTR(std::string, spssColumnLabel) // The name as shown to the user.
	const std::string &spssColumnLabel() const;

	READ_ATTR(std::string, spssRawColName)	// The name as in the file.

	READ_ATTR(size_t, spssStringLen)		// Length of the string (if string).

	READ_ATTR(size_t, columnSpan)			// Number of data cells this column spans.
	void incrementColumnSpan() { _columnSpan++; }

	RW_ATTR(Measure, spssMeasure)			// The Measure from the SPSS file (if any)
	READ_ATTR(FormatTypes, spssFormatType)	// The Format / Type value from the SPSS file.

	READ_ATTR(spss::MissingValueChecker, missingChecker) // A Missing value checker machine.

	RW_ATTR(size_t, charsRemaining)		// The numer of chars (in this case!) remaind to read-in.

	std::vector<double>			numerics;	// Numeric values, one per case.
	std::vector<std::string>	strings;	// String values, one per case.
	LabelByValueDict			spssLables;	// Lables as found in the .SAV

	enum e_celTypeReturn { cellDouble, cellString };
	/**
	 * @brief cellType Gets the cell data type we expect to read for this coloumn.
	 * @return
	 */
	enum e_celTypeReturn cellType() const;


	/**
	 * @brief insert Insert a string into the columns.
	 * @param str String to insert.
	 * @return index of the inserted value (=== case)
	 */
	size_t insert(const std::string &str);

	/**
	 * @brief append Appends a value to the last inserted string.
	 * @param str The string to append.
	 * @return index of the inserted value (=== case)
	 */
	size_t append(const std::string &str);

	/**
	 * @brief getString Gets the string at index.
	 * @param index (== case)
	 * @return The value.
	 */
	const std::string &getString(size_t index) const;

	/**
	 * @brief insert Inserts a double.
	 * @param value The value to insert,
	 * @return index of the inserted value (=== case)
	 */
	size_t insert(double value);


	/**
	 * @brief spssStringLen Set the length of the string (in column).
	 * @param value Value to set.
	 */
	void spssStringLen(size_t value);

	/**
	 * @brief charsRemaining Find the number of chars remaining for this buffer, for one data cell.
	 * @param bufferSzie The size of the buffer.
	 * @return Value
	 */
	long cellCharsRemaining(size_t bufferSize);

	/**
	 * @brief getJaspColumnType Finds the column type that JASP will use.
	 * @return A column type.
	 *
	 */
	Column::ColumnType getJaspColumnType() const;

	/**
	 * @brief format Fomats a number that SPSS holds as a numeric value that JASP cannot deal with.
	 * @param value The value to format.
	 * @param floatInfo The float info record we have.
	 * @return
	 */
	std::string format(double value, const FloatInfoRecord &floatInfo) const;

	/**
	 * @brief containsFraction Returns false if all values are integer.
	 * @param values VAlues to check
	 * @return true if a fractional part found.
	 */
	bool containsFraction() const { return _containsFraction(numerics); }


	protected:
	/**
	 * @brief processStrings Converts any strings in the data fields.
	 * @param dictData The
	 *
	 * Should be implemented in classes where holdStrings maybe or is true.
	 *
	 */
	virtual void processStrings(const CodePageConvert &converter);

	private:
	// Day Zero for spss files.
	static const QDate _beginEpoch;

	/**
	 * @brief _toQDateTime Convert SPSS seconds to a date/time.
	 * @param dt - Target
	 * @param seconds Number of seconds since start of SPSS epoch.
	 * @return void
	 */
	static QDateTime* _asDateTime(double seconds);

	/**
	 * @brief containsFraction Returns false if all values are integer.
	 * @param values VAlues to check
	 * @return true if a fractional part found.
	 */
	static bool _containsFraction(const std::vector<double> &values);

	/*
	 * Help functions for _toDateTime().
	 */
	QString _weekday(unsigned short int wd) const;

	QString _month(unsigned short int mnth) const;
};


class SPSSDictionary : public std::map<size_t, SPSSColumn>
{
public:

	/**
	 * @brief insert Inserts a column.
	 * @param column Column to insert
	 */
	void add(size_t dictIndex, const SPSSColumn &column)
	{ SPSSDictionary::insert( std::pair<size_t, SPSSColumn>(dictIndex, column) ); }

	/**
	 * @brief getColumn Gets a column for an entry number.
	 * @param entry Entry number to fetch
	 * @return Found column.
	 */
	SPSSColumn &getColumn(size_t entry)
	{ return find(entry)->second; }

	SPSSColumn &getLastColumn()
	{ return rend()->second; }
};

/*
 * A vector of SPSSColumns, with an "auto iterator."
 * Also holds the convertors for both numeric endian
 * and string code page convertors.
 */
class SPSSColumns : public SPSSDictionary
{
public:

	typedef std::map<std::string, size_t> LongColsData;

	SPSSColumns();

	/**
	 * @brief resetCols Used after vect::push_back() or similar, reset the next col iterator
	 *
	 */
	void resetCols();

	/**
	 * @brief getColumn Get next column wrapping as required.
	 * @return
	 */
	SPSSColumn& getNextColumn();

	/**
	 * @brief isSpaning
	 * @return True if the last getColumn() call found a contination column.
	 */
	bool isSpaning() const { return _isSpaning; }

	/**
	 * @brief numCases Set the number of cases.
	 * @param num Number of cases to set.
	 */
	void numCases(int32_t num);
	void numCases(int64_t num);

	/**
	 * @brief numCases
	 * @return The numer of cases found (-1 if notknown.)
	 */
	size_t numCases()
	const
	{
		return (size_t) _numCases;
	}

	/**
	 * @brief hasNoCases Checks for number of cases > 0
	 * @return true if no cases held/found.
	 */
	bool hasNoCases() const
	{
		return _numCases < 1L;
	}

	/**
	 * @brief veryLongColsDat Sets the very long strings data.
	 * @param vlcd The value to set.
	 */
	void veryLongColsDat(const LongColsData &vlcd)
	{
		_longColsDta.insert(vlcd.begin(), vlcd.end());
	}

	/**
	 * @brief veryLongColsDat Gets the very long strings data.
	 * @return The value found.
	 */
	const LongColsData &veryLongColsDat() const
	{
		return _longColsDta;
	}

	/**
	 * @brief processStringsPostLoad - Delas with very Long strings (len > 255) and CP processes all strings.
	 * Call after the data is loaded!.
	 */
	void processStringsPostLoad(boost::function<void (const std::string &, int)> progress);

	/**
	 * @brief numericsConv Access the numeric convertor.
	 * @return
	 */
	NumericConverter &numericsConv() { return _numConvert; }

	/**
	 * @brief setStrCnvrtr Sets the string convertor to use.
	 * @param convtr
	 *
	 * N.B. This class takes over ownership of the instance:
	 * @code
		...
		inst.setStrCnvrtr( new SpssCPConvert(ICUConnnector::dos437) );
		...
	   @endcode
	 */
	void setStrCnvrtr(CodePageConvert *convtr)
	{
		_stringConvert.reset(convtr);
	}

	/**
	 * @brief stringsConv Gets the string convertor.
	 * @return String convertoer instance.
	 *
	 * N.B. Will "blow-up" (null pointer exception) if not set!
	 */
	CodePageConvert &stringsConv() { return *_stringConvert.get(); }

private:

	NumericConverter				_numConvert; /** < Numeric Endain fixer. */

	std::auto_ptr<CodePageConvert>	_stringConvert; /** < Code Page convertor. */

	// Data objects used in the getNextColumn iterator.
	SPSSDictionary::iterator	_currentColIter;
	size_t						_remainingColSpan;
	bool						_isSpaning;

	LongColsData	_longColsDta; /** < Very long strings data. */

	int64_t _numCases; /** < Number of cases found to date. */
};

} // end namespace


#undef _ATT_VALUE
#undef READ_ATTR
#undef RW_ATTR

#endif // SPSSRECINTER_H
