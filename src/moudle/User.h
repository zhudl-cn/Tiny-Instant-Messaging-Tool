#pragma once
#include <string>
class User {
public:
	User() = default;
	User(int id, std::string name, std::string pwd, std::string state);

	void setId(int id) { m_id = id; }
	void setName(std::string name) { m_name = name; }
	void setPwd(std::string pwd) { m_password = pwd; }
	void setState(std::string state) { m_state = state; }

	int getId() const { return m_id; }
	std::string getName() const { return m_name; }
	std::string getPwd() const { return m_password; }
	std::string getState() const { return m_state; }

private:
	int             m_id{ -1 };
	std::string     m_name{ "" };
	std::string     m_password{ "" };
	std::string     m_state{ "" };
};

