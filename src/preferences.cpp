/*
This file is part of the NppCrypt Plugin [www.cerberus-design.de] for Notepad++ [ Copyright (C)2003 Don HO <don.h@free.fr> ]

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/


#include "preferences.h"
#include "encoding.h"
#include <fstream>

CPreferences& preferences= CPreferences::Instance();

CPreferences::CPreferences()
{
	keys.resize(1);
	memcpy(keys[0].data, "bJmX/AokoOlC}my.", 16);
	lstrcpy(keys[0].label, TEXT("nppcrypt default"));
	lstrcpy(files.extension, TEXT("nppcrypt"));
	files.askonsave = true;
	files.enable = false;
	files.ext_length = 8;
	no_ascii = false;
};

bool CPreferences::load(const TCHAR* path, Crypt::Options& crypt, Crypt::HashOptions& hash, Crypt::RandOptions& random)
{
	if(!path)
		false;

	#ifdef UNICODE
	Encode::wchar_to_utf8(path, -1, filepath);
	#else
	filepath.assign(path);
	#endif

	std::ifstream f(filepath);
	
	try {
		f.exceptions ( std::ifstream::failbit | std::ifstream::badbit );
		if(!f.is_open())
			throw NULL;
		char theader[22];
		f.read(theader, 21);
		theader[21]=0;
		if(strcmp("nppcrypt.config.v1010", theader) != 0)
			throw NULL;

		bool t_bool;
		size_t t_size_t;
		int t_int;
		KeyPreset t_key;

		// encoding
		f.read(reinterpret_cast<char*>(&t_bool), sizeof(bool));
		Encode::Options::win_line_endings = t_bool ? true : false;
		f.read(reinterpret_cast<char*>(&t_bool), sizeof(bool));
		Encode::Options::hex_spaces = t_bool ? true : false;
		f.read(reinterpret_cast<char*>(&t_bool), sizeof(bool));
		Encode::Options::hex_lowercase = t_bool ? true : false;
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		Encode::Options::hex_values_p_line = (t_size_t < 9999) ? t_size_t : 64;
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		Encode::Options::base64_chars_p_line = (t_size_t < 9999) ? t_size_t : 128;

		// nppcrypt-files
		f.read(reinterpret_cast<char*>(&t_bool), sizeof(bool));
		files.enable = t_bool ? true : false;
		f.read(reinterpret_cast<char*>(&t_bool), sizeof(bool));
		files.askonsave = t_bool ? true : false;
		f.read(reinterpret_cast<char*>(&files.extension), sizeof(files.extension));
		files.extension[30]=0;
		files.ext_length = lstrlen(files.extension);

		// current crypt-options:
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t < static_cast<size_t>(Crypt::Cipher::COUNT)) { crypt.cipher = static_cast<Crypt::Cipher>(t_size_t); }
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t < static_cast<size_t>(Crypt::Mode::COUNT)) { crypt.mode = static_cast<Crypt::Mode>(t_size_t); }
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t < static_cast<size_t>(Crypt::Encoding::COUNT)) { crypt.encoding = static_cast<Crypt::Encoding>(t_size_t); }
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t < static_cast<size_t>(Crypt::InitVector::COUNT)) { crypt.iv = static_cast<Crypt::InitVector>(t_size_t); }
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t < static_cast<size_t>(Crypt::KeyDerivation::COUNT)) { crypt.key.algorithm = static_cast<Crypt::KeyDerivation>(t_size_t); }
		f.read(reinterpret_cast<char*>(&t_int), sizeof(int));
		if(t_int > -2 && t_int <= Crypt::Constants::salt_bytes_max) { crypt.key.salt_bytes = t_int; }
		f.read(reinterpret_cast<char*>(&crypt.key.option1), sizeof(int));
		f.read(reinterpret_cast<char*>(&crypt.key.option2), sizeof(int));
		f.read(reinterpret_cast<char*>(&crypt.key.option3), sizeof(int));
		switch(crypt.key.algorithm) {
		case Crypt::KeyDerivation::pbkdf2:
			if(crypt.key.option1 < 0 || crypt.key.option1 >= static_cast<int>(Crypt::Hash::sha3_256))
				crypt.key.option1 = Crypt::Constants::pbkdf2_default_hash;
			if(crypt.key.option2 < Crypt::Constants::pbkdf2_iter_min || crypt.key.option2 > Crypt::Constants::pbkdf2_iter_max)
				crypt.key.option2 = Crypt::Constants::pbkdf2_iter_default;
			crypt.key.option3 = 0;
			break;
		case Crypt::KeyDerivation::bcrypt:
			if(crypt.key.option1 < Crypt::Constants::bcrypt_iter_min || crypt.key.option1 > Crypt::Constants::bcrypt_iter_max)
				crypt.key.option1 = Crypt::Constants::bcrypt_iter_default;
			crypt.key.option2 = 0;
			crypt.key.option3 = 0;
			break;
		case Crypt::KeyDerivation::scrypt:
			if(crypt.key.option1 < Crypt::Constants::scrypt_N_min || crypt.key.option1 > Crypt::Constants::scrypt_N_max)
				crypt.key.option1 = Crypt::Constants::scrypt_N_default;
			if(crypt.key.option2 < Crypt::Constants::scrypt_r_min || crypt.key.option2 > Crypt::Constants::scrypt_r_max)
				crypt.key.option2 = Crypt::Constants::scrypt_r_default;
			if(crypt.key.option3 < Crypt::Constants::scrypt_p_min || crypt.key.option3 > Crypt::Constants::scrypt_p_max)
				crypt.key.option3 = Crypt::Constants::scrypt_p_default;
			break;
		}

		f.read(reinterpret_cast<char*>(&t_bool), sizeof(bool));
		crypt.hmac.enable = t_bool ? true : false;
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t < static_cast<size_t>(Crypt::Hash::COUNT)) { crypt.hmac.hash = static_cast<Crypt::Hash>(t_size_t); }
		f.read(reinterpret_cast<char*>(&t_int), sizeof(int));
		if(t_int > -2 && t_int < (int)keys.size()) { crypt.hmac.key_id = t_int; }

		// current hash-options:
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t < static_cast<size_t>(Crypt::Hash::COUNT)) { hash.algorithm = static_cast<Crypt::Hash>(t_size_t); }
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t < static_cast<size_t>(Crypt::Encoding::COUNT)) { hash.encoding = static_cast<Crypt::Encoding>(t_size_t); }
		f.read(reinterpret_cast<char*>(&t_bool), sizeof(bool));
		hash.use_key = t_bool ? true : false;
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t > 64)
			throw CExc(CExc::preferences, __LINE__);
		hash.key.resize(t_size_t);
		f.read(&hash.key[0], t_size_t);

		// current random-options:
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t < static_cast<size_t>(Crypt::RandomMode::COUNT)) { random.mode = static_cast<Crypt::RandomMode>(t_size_t); }
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		if(t_size_t <= Crypt::Constants::rand_char_max) { random.length = t_size_t; }

		// key presets:		
		f.read(reinterpret_cast<char*>(&t_size_t), sizeof(size_t));
		keys.reserve(t_size_t);
		for(size_t i=0; i<t_size_t; i++) {
			f.read(reinterpret_cast<char*>(&t_key), sizeof(KeyPreset));
			t_key.label[30]=0;
			keys.push_back(t_key);
		}

	} catch(...) {
		if(f.is_open())
			f.close();
		return false;
	}
	f.close();
	return true;
}

bool CPreferences::save(Crypt::Options& crypt, Crypt::HashOptions& hash, Crypt::RandOptions& random)
{
	/* no human readable config-file, because its easier and faster this way and i don't think it matters */

	std::ofstream f(filepath.c_str(), std::ios::out|std::ios::binary);
	try {
		if(!f.is_open())
			throw NULL;
		f.write("nppcrypt.config.v1010", 21);

		size_t ts;

		// encoding
		f.write(reinterpret_cast<char*>(&Encode::Options::win_line_endings), sizeof(bool));
		f.write(reinterpret_cast<char*>(&Encode::Options::hex_spaces), sizeof(bool));
		f.write(reinterpret_cast<char*>(&Encode::Options::hex_lowercase), sizeof(bool));
		f.write(reinterpret_cast<char*>(&Encode::Options::hex_values_p_line), sizeof(size_t));
		f.write(reinterpret_cast<char*>(&Encode::Options::base64_chars_p_line), sizeof(size_t));
		// nppcrypt-files
		f.write(reinterpret_cast<char*>(&files.enable), sizeof(bool));
		f.write(reinterpret_cast<char*>(&files.askonsave), sizeof(bool));
		f.write(reinterpret_cast<char*>(&files.extension), sizeof(files.extension));
		// current crypt-options:
		f.write(reinterpret_cast<char*>(&crypt.cipher), sizeof(Crypt::Cipher));
		f.write(reinterpret_cast<char*>(&crypt.mode), sizeof(Crypt::Mode));
		f.write(reinterpret_cast<char*>(&crypt.encoding), sizeof(Crypt::Encoding));
		f.write(reinterpret_cast<char*>(&crypt.iv), sizeof(Crypt::InitVector));
		f.write(reinterpret_cast<char*>(&crypt.key.algorithm), sizeof(Crypt::KeyDerivation));
		f.write(reinterpret_cast<char*>(&crypt.key.salt_bytes), sizeof(int));
		f.write(reinterpret_cast<char*>(&crypt.key.option1), sizeof(int));
		f.write(reinterpret_cast<char*>(&crypt.key.option2), sizeof(int));
		f.write(reinterpret_cast<char*>(&crypt.key.option3), sizeof(int));
		f.write(reinterpret_cast<char*>(&crypt.hmac.enable), sizeof(bool));
		f.write(reinterpret_cast<char*>(&crypt.hmac.hash), sizeof(Crypt::Hash));
		f.write(reinterpret_cast<char*>(&crypt.hmac.key_id), sizeof(int));
		// current hash-options:
		f.write(reinterpret_cast<char*>(&hash.algorithm), sizeof(Crypt::Hash));
		f.write(reinterpret_cast<char*>(&hash.encoding), sizeof(Crypt::Encoding));
		f.write(reinterpret_cast<char*>(&hash.use_key), sizeof(bool));
		ts = hash.key.size();
		f.write(reinterpret_cast<char*>(&ts), sizeof(size_t));
		f.write(&hash.key[0], ts);
		// current random-options:
		f.write(reinterpret_cast<char*>(&random.mode), sizeof(Crypt::RandomMode));
		f.write(reinterpret_cast<char*>(&random.length), sizeof(size_t));
		// key presets:
		ts = keys.size() - 1;
		f.write(reinterpret_cast<char*>(&ts), sizeof(size_t));
		for(size_t i=1; i<keys.size(); i++) {
			f.write(reinterpret_cast<char*>(&keys[i]), sizeof(KeyPreset));
		}
	} catch(...) {
		if(f.is_open())
			f.close();
		return false;
	}
	f.close();
	return true;
}

size_t CPreferences::getKeyNum()
{
	return keys.size();
}

bool CPreferences::addKey(const KeyPreset& key)
{
	if(!lstrlen(key.label) || keys.size() >= 20)
		return false;
	keys.push_back(key);
	return true;
}

bool CPreferences::delKey(size_t i)
{
	if(i < keys.size()) {
		keys.erase(keys.begin()+i);
		return true;
	} else {
		return false;
	}
}

const TCHAR* CPreferences::getKeyLabel(size_t i)
{
	if(i < keys.size())
		return keys[i].label;
	else
		return NULL;
}

const unsigned char* CPreferences::getKey(size_t i)
{
	if(i < keys.size())
		return keys[i].data;
	else
		return NULL;
}
