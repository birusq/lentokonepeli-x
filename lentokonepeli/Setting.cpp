#include "Setting.h"

bool Setting::iniValExists(std::string section, std::string key) {
	if (ini->GetValue(section.c_str(), key.c_str(), "*") == "*") {
		return false;
	}
	return true;
}


IntSetting::IntSetting(CSimpleIniA* ini_, std::string section_, std::string key_, int default_) :Setting(ini_, section_, key_) {
	if (!iniValExists(section_, key_)) {
		setValue(default_);
		value = default_;
	}
	value = (int)ini->GetLongValue(section_.c_str(), key_.c_str());
}

void IntSetting::setValue(int val) {
	value = val;
	ini->SetLongValue(section.c_str(), key.c_str(), val, (const char*)0, false, true);
	ini->SaveFile("config.ini");
}


FloatSetting::FloatSetting(CSimpleIniA* ini_, std::string section_, std::string key_, float default_) :Setting(ini_, section_, key_) {
	if (!iniValExists(section_, key_)) {
		setValue(default_);
		value = default_;
	}
	value = (float)ini->GetDoubleValue(section_.c_str(), key_.c_str());
}

void FloatSetting::setValue(float val) {
	value = val;
	ini->SetDoubleValue(section.c_str(), key.c_str(), val, (const char*)0, true);
	ini->SaveFile("config.ini");
}


BoolSetting::BoolSetting(CSimpleIniA* ini_, std::string section_, std::string key_, bool default_) :Setting(ini_, section_, key_) {
	if (!iniValExists(section_, key_)) {
		setValue(default_);
		value = default_;
	}
	value = ini->GetBoolValue(section_.c_str(), key_.c_str());
}

void BoolSetting::setValue(bool val) {
	value = val;
	ini->SetBoolValue(section.c_str(), key.c_str(), val, (const char*)0, true);
	ini->SaveFile("config.ini");
}


StringSetting::StringSetting(CSimpleIniA* ini_, std::string section_, std::string key_, std::string default_, unsigned int maxLength_) :Setting(ini_, section_, key_), maxLength{maxLength_} {
	if (!iniValExists(section_, key_)) {
		setValue(default_);
		value = default_;
	}
	value = ini->GetValue(section_.c_str(), key_.c_str());

	if (value.size() > maxLength) {
		value = value.substr(0, maxLength);
	}
}

void StringSetting::setValue(std::string val) {
	value = val;
	ini->SetValue(section.c_str(), key.c_str(), val.c_str(), (const char*)0, true);
	ini->SaveFile("config.ini");
}


KeySetting::KeySetting(CSimpleIniA* ini_, std::string section_, std::string key_, sf::Keyboard::Key default_) :Setting(ini_, section_, key_) {
	if (!iniValExists(section_, key_)) {
		setValue(default_);
		value = default_;
	}
	value = thor::toKeyboardKey(ini->GetValue(section_.c_str(), key_.c_str()));
}

void KeySetting::setValue(sf::Keyboard::Key val) {
	value = val;
	ini->SetValue(section.c_str(), key.c_str(), thor::toString(val).c_str(), (const char*)0, true);
	ini->SaveFile("config.ini");
}