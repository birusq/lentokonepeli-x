#pragma once
#include <string>
#include <Thor\Input.hpp>
#include <SFML\System.hpp>
#include "SimpleIni.h"

class Setting {
public:
	Setting() {}
	Setting(CSimpleIniA* ini_, std::string section_, std::string key_) :ini{ ini_ }, section{ section_ }, key{ key_ } {}
protected:
	bool Setting::iniValExists(std::string section, std::string key);

	std::string section;
	std::string key;
	CSimpleIniA* ini;
};

class IntSetting : Setting {
public:
	IntSetting() {}
	IntSetting(CSimpleIniA* ini_, std::string section_, std::string key_, int default_);

	int getValue() { return value; }
	void setValue(int val);

	operator int() const { return value; }
private:
	int value;
};

class FloatSetting : Setting {
public:
	FloatSetting() {}
	FloatSetting(CSimpleIniA* ini_, std::string section_, std::string key_, float default_);

	float getValue() { return value; }
	void setValue(float val);

	operator float() const { return value; }
private:
	float value;
};

class BoolSetting : Setting {
public:
	BoolSetting() {}
	BoolSetting(CSimpleIniA* ini_, std::string section_, std::string key_, bool default_);

	bool getValue() { return value; }
	void setValue(bool val);

	operator bool() const { return value; }
private:
	bool value;
};

class StringSetting : Setting {
public:
	StringSetting() {}
	StringSetting(CSimpleIniA* ini_, std::string section_, std::string key_, std::string default_);

	std::string getValue() { return value; }
	void setValue(std::string val);

	operator std::string() const { return value; }
private:
	std::string value;
};

class KeySetting : Setting {
public:
	KeySetting() {}
	KeySetting(CSimpleIniA* ini_, std::string section_, std::string key_, sf::Keyboard::Key default_);

	sf::Keyboard::Key getValue() { return value; }
	void setValue(sf::Keyboard::Key val);

	operator sf::Keyboard::Key() const { return value; }
private:
	sf::Keyboard::Key value;
};