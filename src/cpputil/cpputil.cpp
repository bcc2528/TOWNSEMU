/* LICENSE>>
Copyright 2020 Soji Yamakawa (CaptainYS, http://www.ysflight.com)

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

<< LICENSE */
#include <fstream>

#ifdef _WIN32
	#include <direct.h>
	#define getcwd _getcwd
#else
	#include <unistd.h>
#endif

#include "cpputil.h"



std::string cpputil::MakeFullPathName(std::string dirName,std::string fName)
{
	if(dirName.back()=='\\' || dirName.back()=='/')
	{
		return dirName+fName;
	}
	else
	{
		return dirName+"/"+fName;
	}
}

bool cpputil::FileExists(std::string fName)
{
	std::ifstream fp(fName,std::ifstream::binary);
	if(true==fp.is_open())
	{
		fp.close();
		return true;
	}
	return false;
}

std::string cpputil::FindFileWithSearchPaths(std::string fName,const std::vector <std::string> &searchPaths)
{
	if(true==FileExists(fName))
	{
		return fName;
	}

	for(auto path : searchPaths)
	{
		auto ful=MakeFullPathName(path,fName);
		if(true==FileExists(ful))
		{
			return ful;
		}
	}

	std::string inPath,inFile;
	SeparatePathFile(inPath,inFile,fName);
	for(auto path : searchPaths)
	{
		auto ful=MakeFullPathName(path,inFile);
		if(true==FileExists(ful))
		{
			return ful;
		}
	}

	return "";
}

std::vector <unsigned char> cpputil::ReadBinaryFile(std::string fName)
{
	std::vector <unsigned char> dat;
	std::ifstream fp(fName,std::ifstream::binary);
	if(true==fp.is_open())
	{
		fp.seekg(0,fp.end);
		auto length=fp.tellg();
		fp.seekg(0,fp.beg);

		dat.resize(length);
		fp.read((char *)dat.data(),length);

		fp.close();
	}
	return dat;
}

std::vector <unsigned char> cpputil::ReadBinaryFile(std::string fName,long long int start,long long int length)
{
	std::vector <unsigned char> dat;
	std::ifstream fp(fName,std::ifstream::binary);
	if(true==fp.is_open())
	{
		fp.seekg(start,fp.beg);

		dat.resize(length);
		fp.read((char *)dat.data(),length);

		fp.close();
	}
	return dat;
}

bool cpputil::WriteBinaryFile(const std::string &fName,unsigned long long length,const unsigned char dat[])
{
	std::ofstream fp(fName,std::ofstream::binary);
	if(true==fp.is_open())
	{
		fp.write((char *)dat,length);
		fp.close();
		return true;
	}
	return false;
}

bool cpputil::WriteBinaryFile(const std::string &fName,unsigned long long int start,unsigned long long length,const unsigned char dat[])
{
	std::fstream fp(fName,std::ios::binary|std::ios::in|std::ios::out);
	if(true==fp.is_open())
	{
		fp.seekg(start,fp.beg);
		fp.write((char *)dat,length);
		fp.close();
		return true;
	}
	return false;
}

std::vector <std::string> cpputil::ReadTextFile(std::string fName)
{
	std::vector <std::string> text;
	std::ifstream fp(fName);
	if(true==fp.is_open())
	{
		while(true!=fp.eof())
		{
			std::string str;
			std::getline(fp,str);
			text.push_back(str);
		}
	}
	return text;
}

bool cpputil::WriteTextFile(std::string fName,const std::vector <std::string> &text)
{
	std::ofstream fp(fName);
	if(true==fp.is_open())
	{
		for(auto str : text)
		{
			fp << str << "\n";
		}
		return true;
	}
	return false;
}

std::vector <std::string> cpputil::Parser(const char str[])
{
	const int STATE_OUTSIDE=0,STATE_WORD=1,STATE_DOUBLEQUOTE=2;
	const char DQ='\"';
	int state=STATE_OUTSIDE;
	std::string curStr;
	std::vector <std::string> argv;
	for(int i=0; 0!=str[i]; ++i)
	{
		if(STATE_OUTSIDE==state)
		{
			if(DQ==str[i])
			{
				curStr="";
				state=STATE_DOUBLEQUOTE;
			}
			else if(' '!=str[i] && '\t'!=str[i] && '\n'!=str[i] && '\r'!=str[i])
			{
				curStr.push_back(str[i]);
				state=STATE_WORD;
			}

		}
		else if(STATE_WORD==state)
		{
			if(' '==str[i] || '\t'==str[i] || '\n'==str[i] || '\r'==str[i] || 0==str[i+1])
			{
				if(' '!=str[i] && '\t'!=str[i] && '\n'!=str[i] && '\r'!=str[i])
				{
					curStr.push_back(str[i]);
				}
				argv.push_back((std::string &&)curStr);
				curStr="";
				state=STATE_OUTSIDE;
			}
			else
			{
				curStr.push_back(str[i]);
			}
		}
		else if(STATE_DOUBLEQUOTE==state)
		{
			if(DQ==str[i])
			{
				argv.push_back((std::string&&)curStr);
				curStr="";
				state=STATE_OUTSIDE;
			}
			else
			{
				curStr.push_back(str[i]);
			}
		}
	}

	if(0<curStr.size())
	{
		argv.push_back((std::string &&)curStr);
	}

	return argv;
}

const std::string &cpputil::Capitalize(std::string &s)
{
	for(auto &c : s)
	{
		if('a'<=c && c<='z')
		{
			c=c+'A'-'a';
		}
	}
	return s;
}

char cpputil::Capitalize(char in)
{
	if('a'<=in && in<='z')
	{
		return in+'A'-'a';
	}
	return in;
}

std::string cpputil::Capitalize(const char s[])
{
	return std::string(s);
}

std::string cpputil::GetExtension(std::string fName)
{
	if(""==fName)
	{
		return "";
	}

	int lastDot=-1;
	for(int i=0; 0!=fName[i]; ++i)
	{
		if('.'==fName[i])
		{
			lastDot=i;
		}
	}

	if(0<=lastDot)
	{
		std::string ext(fName.data()+lastDot);
		return ext;
	}
	return "";
}

std::string cpputil::ChangeExtension(const char orgFName[],const char newExt[])
{
	int lastDot=-1;
	for(int i=0; 0!=orgFName[i]; ++i)
	{
		if('.'==orgFName[i])
		{
			lastDot=i;
		}
	}

	std::string newFn;

	if(0>lastDot)
	{
		newFn=orgFName;
	}
	else
	{
		for(int i=0; i<lastDot; ++i)
		{
			newFn.push_back(orgFName[i]);
		}
	}

	for(int i=0; 0!=newExt[i]; ++i)
	{
		newFn.push_back(newExt[i]);
	}

	return newFn;
}

std::string cpputil::RemoveExtension(const char orgFName[])
{
	int lastDot=-1;
	for(int i=0; 0!=orgFName[i]; ++i)
	{
		if('.'==orgFName[i])
		{
			lastDot=i;
		}
	}

	std::string newFn;

	if(0>lastDot)
	{
		newFn=orgFName;
	}
	else
	{
		for(int i=0; i<lastDot; ++i)
		{
			newFn.push_back(orgFName[i]);
		}
	}

	return newFn;
}

std::string cpputil::GetBaseName(const std::string &path)
{
	if(0==path.size())
	{
		return "";
	}

	int lastSlash=0;
	for(int i=0; path[i]!=0; ++i)
	{
		if(':'==path[i] || '/'==path[i] || '\\'==path[i])
		{
			lastSlash=i;
		}
	}

	std::string basename;
	for(int i=lastSlash+1; 0!=path[i]; ++i)
	{
		basename.push_back(path[i]);
	}

	return basename;
}

bool cpputil::StrStartsWith(const std::string &str,const char ptn[])
{
	int i;
	for(i=0; i<str.size() && 0!=ptn[i]; ++i)
	{
		if(str[i]!=ptn[i])
		{
			return false;
		}
	}
	if(0==ptn[i])
	{
		return true;
	}
	return false;
}

const char *cpputil::StrSkip(const char str[],const char ptn[])
{
	for(int i=0; 0!=str[i]; ++i)
	{
		if(str[i]==ptn[0])
		{
			int j;
			for(j=0; 0!=str[i+j] && 0!=ptn[j]; ++j)
			{
				if(str[i+j]!=ptn[j])
				{
					goto NEXTI;
				}
			}
			if(0==ptn[j])
			{
				return str+i+j;
			}
		}
	NEXTI:
		;
	}
	return nullptr;
}

const char *cpputil::StrSkipSpace(const char str[])
{
	int i;
	for(i=0; 0!=str[i]; ++i)
	{
		if(' '!=str[i] && '\t'!=str[i])
		{
			return str+i;
		}
	}
	return str+i;
}

const char *cpputil::StrSkipNonSpace(const char str[])
{
	int i;
	for(i=0; 0!=str[i]; ++i)
	{
		if(' '==str[i] || '\t'==str[i])
		{
			return str+i;
		}
	}
	return str+i;
}

const char *cpputil::BoolToStr(bool b)
{
	if(b==true)
	{
		return "TRUE";
	}
	else
	{
		return "FALSE";
	}
}

bool cpputil::StrToBool(const std::string &str)
{
	return "TRUE"==str || "True"==str || "true"==str;
}

int cpputil::Xtoi(const char str[])
{
	int n=0;
	while(0!=str[0])
	{
		if('0'<=*str && *str<='9')
		{
			n*=16;
			n+=(*str-'0');
		}
		else if('a'<=*str && *str<='f')
		{
			n*=16;
			n+=(10+*str-'a');
		}
		else if('A'<=*str && *str<='F')
		{
			n*=16;
			n+=(10+*str-'A');
		}
		else
		{
			break;
		}
		++str;
	}
	return n;
}

int cpputil::Atoi(const char str[])
{
	if(nullptr==str)
	{
		return 0;
	}

	int sign=1;
	if('-'==str[0])
	{
		sign=-1;
		++str;
	}

	int n=0;
	if(('0'==str[0] && ('x'==str[1] || 'X'==str[1])) ||
	   ('&'==str[0] && ('h'==str[1] || 'H'==str[1])))
	{
		n=Xtoi(str+2);
	}
	else if('$'==str[0])
	{
		n=Xtoi(str+1);
	}
	else
	{
		bool hexaDecimal=false;
		for(int i=0; str[i]!=0; ++i)
		{
			if(('h'==str[i] || 'H'==str[i]) &&
			   (str[i+1]<'a' || 'z'<str[i+1]) &&
			   (str[i+1]<'A' || 'Z'<str[i+1]) &&
			   (str[i+1]<'0' || '9'<str[i+1]))
			{
				hexaDecimal=true;
				break;
			}
		}
		if(true==hexaDecimal)
		{
			n=Xtoi(str);
		}
		else
		{
			while(0!=*str)
			{
				if('0'<=*str && *str<='9')
				{
					n*=10;
					n+=(*str-'0');
				}
				else
				{
					break;
				}
				++str;
			}
		}
	}
	return n*sign;
}

inline char cpputil::FourBitToX(int i)
{
	if(0<=i && i<=9)
	{
		return '0'+i;
	}
	else
	{
		return 'A'+i-10;
	}
}

void cpputil::SeparatePathFile(std::string &path,std::string &file,const std::string &fName)
{
	long long int lastSeparator=-1;
	for(long long int i=0; 0!=fName[i]; ++i)
	{
		if('/'==fName[i] || ':'==fName[i] || '\\'==fName[i])
		{
			lastSeparator=i;
		}
	}

	path=fName;
	path.resize(lastSeparator+1);
	file=fName.data()+lastSeparator+1;
}

std::string cpputil::Getcwd(void)
{
	char cwdIn[1024];
	getcwd(cwdIn,1023);

	std::string cwd=cwdIn;
	// Curse backslash.  MSDOS will forever be laughed at for backslash.  (Oh, and also 0D 0A.)
	for(auto &c : cwd)
	{
		if('\\'==c)
		{
			c='/';
		}
	}

	return cwd;
}

bool cpputil::IsRelativePath(std::string path)
{
	if(0<path.size() && ('/'==path[0] || '\\'==path[0]))
	{
		return false;
	}
#ifdef _WIN32
	if(3<=path.size() && ':'==path[1] && ('/'==path[2] || '\\'==path[2]))
	{
		return false;
	}
#endif
	return true;
}

std::string cpputil::TrueName(std::string incoming)
{
	// Just curse backslash.
	for(auto &c : incoming)
	{
		if('\\'==c)
		{
			c='/';
		}
	}

	std::string path=incoming;
	std::string cwd=Getcwd();


#ifdef _WIN32
	int incomingDrive=-1,currentDrive=-1;

	// Is full-path given?
	if(3<=incoming.size() && ':'==incoming[1] && '/'==incoming[2])
	{
		if('a'<=incoming[0] && incoming[0]<='z')
		{
			incoming[0]+='A'-'a';
		}
		return incoming;
	}

	if(2<=cwd.size() && cwd[1]==':')
	{
		currentDrive=cwd[0];
		if('a'<=currentDrive && currentDrive<='z')
		{
			currentDrive-='a';
			cwd[0]+='A'-'a';
		}
		else
		{
			currentDrive-='A';
		}
	}

	if(2<=incoming.size() && incoming[1]==':')
	{
		incomingDrive=incoming[0];
		if('a'<=incomingDrive && incomingDrive<='z')
		{
			incomingDrive-='a';
		}
		else
		{
			incomingDrive-='A';
		}
	}

	if(-1!=incomingDrive && incomingDrive!=currentDrive)
	{
		// If the current drive!=incoming drive, no way.
		// Just curse the drive letter.
		return path;
	}

	if(-1!=currentDrive && '/'==incoming[0])
	{
		std::string driveLetter;
		driveLetter.push_back('A'+currentDrive);
		driveLetter.push_back(':');
		return driveLetter+incoming;
	}
#else
	if('/'==incoming[0])
	{
		return incoming;
	}
#endif

	// Incoming is most likely is a relative path.

	if('/'!=cwd.back())
	{
		cwd.push_back('/');
	}
	path=cwd+incoming;

	SimplifyPath(path);

	return path;
}

std::string cpputil::MakeRelativePath(std::string fName,std::string relativeToThisDir)
{
	fName=cpputil::TrueName(fName);
	relativeToThisDir=cpputil::TrueName(relativeToThisDir);

	if(relativeToThisDir.size()==0)
	{
		return fName;
	}
	if(0<relativeToThisDir.size() && relativeToThisDir.back()=='/')
	{
		relativeToThisDir.pop_back();
	}
	if(relativeToThisDir.size()==0)
	{
		return fName;
	}

	// relativeToThisDir="/abc/xyz/pqr"
	// fName=            "/abc/subdir/file"
	// Return=           "../../subdir/file"

	int currentSlash=0;
	while(currentSlash<relativeToThisDir.size())
	{
		auto nextSlash=currentSlash;
		if(relativeToThisDir[nextSlash]=='/')
		{
			++nextSlash;
		}
		while(nextSlash<relativeToThisDir.size() && relativeToThisDir[nextSlash]!='/')
		{
			++nextSlash;
		}

		bool match=true;
		for(int i=currentSlash; i<nextSlash; ++i)
		{
			auto a=relativeToThisDir[i];
			auto b=fName[i];
		#ifdef _WIN32
			if('a'<=a && a<='z')
			{
				a+='A'-'a';
			}
			if('a'<=b && b<='z')
			{
				b+='A'-'a';
			}
		#endif
			if(a!=b)
			{
				match=false;
				break;
			}
		}

		if(true==match)
		{
			currentSlash=nextSlash;
		}
		else
		{
			break;
		}
	}

	// relativeToThisDir="/abc/xyz/pqr"
	// fName=            "/abc/subdir/file"
	//                        ^ currentSlash points to.

	std::string relPath;
	for(int i=currentSlash; i<relativeToThisDir.size(); ++i)
	{
		if('/'==relativeToThisDir[i])
		{
			relPath=relPath+"../";
		}
	}
	for(int i=currentSlash+1; i<fName.size(); ++i)
	{
		relPath.push_back(fName[i]);
	}

	return relPath;
}

void cpputil::SimplifyPath(std::string &path)
{
	std::vector <std::string> pathBreakDown;
	char separator='/';
	{
		int state=0;
		std::string current;
		for(auto c : path)
		{
			if(0==state)
			{
				if(c=='/' || c=='\\')
				{
					separator=c;
					pathBreakDown.push_back((std::string &&)current);
					current="";
					state=1;
				}
				else
				{
					current.push_back(c);
				}
			}
			else if(1==state)
			{
				if('/'!=c && '\\'!=c)
				{
					current.push_back(c);
					state=0;
				}
			}
		}
		if(""!=current)
		{
			pathBreakDown.push_back((std::string &&)current);
		}
	}
	int i=0;
	while(i<pathBreakDown.size())
	{
		if("."==pathBreakDown[i])
		{
			pathBreakDown.erase(pathBreakDown.begin()+i);
		}
		else if(".."==pathBreakDown[i] && 0<i)
		{
			pathBreakDown.erase(pathBreakDown.begin()+i-1);
			pathBreakDown.erase(pathBreakDown.begin()+i-1);
			--i;
		}
		else
		{
			++i;
		}
	}

	path="";
	for(auto &p : pathBreakDown)
	{
		path+=p;
		path.push_back(separator);
	}
	if(""!=path)
	{
		path.pop_back();
	}
}

long long int cpputil::FileSize(const std::string &fName)
{
	std::vector <unsigned char> dat;
	std::ifstream fp(fName,std::ifstream::binary);
	if(true==fp.is_open())
	{
		fp.seekg(0,fp.end);
		auto length=fp.tellg();
		fp.close();
		return length;
	}
	return 0;
}

char BoolToChar(bool f)
{
	return (true==f ? '1' : '0');
}

std::string cpputil::Uitox(unsigned int i)
{
	std::string s;
	s.resize(8);
	for(int n=7; 0<=n; --n)
	{
		s[n]=FourBitToX(i&0x0F);
		i>>=4;
	}
	return s;
}

std::string cpputil::Ustox(unsigned short i)
{
	std::string s;
	s.resize(4);
	for(int n=3; 0<=n; --n)
	{
		s[n]=FourBitToX(i&0x0F);
		i>>=4;
	}
	return s;
}

std::string cpputil::Ubtox(unsigned char i)
{
	std::string s;
	s.resize(2);
	for(int n=1; 0<=n; --n)
	{
		s[n]=FourBitToX(i&0x0F);
		i>>=4;
	}
	return s;
}

std::string cpputil::Uitoa(unsigned int i)
{
	if(0==i)
	{
		return "0";
	}

	std::string s;
	while(0<i)
	{
		s.push_back('0'+(i%10));
		i/=10;
	}
	for(int n=0; n<s.size()/2; ++n)
	{
		std::swap(s[n],s[s.size()-1-n]);
	}
	return s;
}

std::string cpputil::Uitoa(unsigned int i,unsigned minLen)
{
	std::string s=Itoa(i);
	if(s.size()<minLen)
	{
		std::string space;
		for(auto i=s.size(); i<minLen; ++i)
		{
			space.push_back(' ');
		}
		return space+s;
	}
	return s;
}

std::string cpputil::UitoaZeroPad(unsigned int i,unsigned numDigits)
{
	std::string str;
	for(int d=0; d<numDigits; ++d)
	{
		str.push_back('0');
	}
	int d=numDigits-1;
	while(0!=i && 0<=d)
	{
		str[d]='0'+i%10;
		i/=10;
		--d;
	}
	return str;
}

std::string cpputil::Itox(int i)
{
	if(0<=i)
	{
		return Uitox(i);
	}
	else
	{
		std::string sgnd="-";
		return sgnd+Uitox(-i);
	}
}

std::string cpputil::Stox(short i)
{
	if(0<=i)
	{
		return Ustox(i);
	}
	else
	{
		std::string sgnd="-";
		return sgnd+Ustox(-i);
	}
}

std::string cpputil::Btox(char i)
{
	if(0<=i)
	{
		return Ubtox(i);
	}
	else
	{
		std::string sgnd="-";
		return sgnd+Ubtox(-i);
	}
}

std::string cpputil::Itoa(int i)
{
	if(0<=i)
	{
		return Uitoa(i);
	}
	else
	{
		std::string sgnd="-";
		return sgnd+Uitoa(-i);
	}
}

std::string cpputil::Itoa(int i,unsigned minLen)
{
	std::string s=Itoa(i);
	if(s.size()<minLen)
	{
		std::string space;
		for(auto i=s.size(); i<minLen; ++i)
		{
			space.push_back(' ');
		}
		return space+s;
	}
	return s;
}

void cpputil::ExtendString(std::string &str,int minimumLength)
{
	while(str.size()<minimumLength)
	{
		str.push_back(' ');
	}
}

char cpputil::BoolToChar(bool flag)
{
	return ((true==flag) ? '1' : '0');
}

const char *cpputil::BoolToNumberStr(bool flag)
{
	return ((true==flag) ? "1" : "0");
}

std::vector <std::string> cpputil::MakeDump(unsigned int printAddr,long long int length,const unsigned char data[])
{
	// Make it 32-bit addressing.  I don't think there is any point rounding the address for memory dump.
	const int addressSize=32;
	std::vector <std::string> text;

	auto lineStart=(printAddr&~0x0F);
	auto lineEnd=((printAddr+length-1)&~0x0F);

	for(auto addr0=lineStart; addr0<=lineEnd; addr0+=16)
	{
		std::string str;

		// printAddr corresponds to data[0]

		str+=cpputil::Uitox(addr0);
		for(int i=0; i<16; ++i)
		{
			auto addr=addr0+i;
			if(addr<printAddr || printAddr+length<=addr)
			{
				str+="   ";
			}
			else
			{
				str+=" "+cpputil::Ubtox(data[addr-printAddr]);
			}
		}
		str.push_back('|');
		for(int i=0; i<16; ++i)
		{
			auto addr=addr0+i;
			if(addr<printAddr || printAddr+length<=addr)
			{
				str.push_back(' ');
			}
			else
			{
				auto byte=data[addr-printAddr];
				if(byte<' ' || 0x80<=byte)
				{
					str.push_back(' ');
				}
				else
				{
					str.push_back(byte);
				}
			}
		}
		text.push_back((std::string &&)str);
	}

	return text;
}

bool cpputil::WildCardCompare(std::string ptn,std::string str)
{
	return WildCardCompare(ptn.size(),ptn.data(),str.size(),str.data());
}

bool cpputil::WildCardCompare(int lenPtn,const char ptn[],int lenStr,const char str[])
{
	while(0<lenPtn && 0<lenStr)
	{
		if(*ptn==*str || '?'==*ptn)
		{
			++ptn;
			++str;
			--lenPtn;
			--lenStr;
			if(0==lenStr && 0==lenPtn)
			{
				return true;
			}
		}
		else if('*'==*ptn)
		{
			// Any part after ptn may match any part after str.
			++ptn;
			--lenPtn;
			if(0==lenPtn)
			{
				return true;
			}
			while(0<lenStr)
			{
				if(true==WildCardCompare(lenPtn,ptn,lenStr,str))
				{
					return true;
				}
				--lenStr;
				++str;
			}
		}
		else
		{
			return false;
		}
	}
	return false;
}

bool cpputil::StrIncludes(const std::string str,char c)
{
	for(auto x : str)
	{
		if(x==c)
		{
			return true;
		}
	}
	return false;
}

bool cpputil::Is2toN(unsigned int i)
{
	return 0==(i&(i-1));
}

std::string cpputil::ExpandFileName(std::string src,const std::map <std::string,std::string> &dict)
{
	std::string expandedName,pending,variable;
	unsigned int state=0; // 0:Nothing  1:Found Found '$'  2:Found '{' immediately afeter '$'
	for(auto c : src)
	{
		switch(state)
		{
		case 0:
			if('$'==c)
			{
				pending.push_back(c);
				state=1;
			}
			else
			{
				expandedName.push_back(c);
			}
			break;
		case 1:
			if('{'==c)
			{
				pending.push_back(c);
				state=2;
			}
			else
			{
				expandedName+=pending;
				pending="";
				variable="";
				expandedName.push_back(c);
				state=0;
			}
			break;
		case 2:
			if('}'==c)
			{
				auto found=dict.find(variable);
				if(dict.end()!=found)
				{
					expandedName+=found->second;
				}
				else
				{
					expandedName+=pending;
					expandedName.push_back(c);
				}
				pending="";
				variable="";
				state=0;
			}
			else
			{
				pending.push_back(c);
				variable.push_back(c);
			}
			break;
		}
	}
	expandedName+=pending;
	return expandedName;
}
