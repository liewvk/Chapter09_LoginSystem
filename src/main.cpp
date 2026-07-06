#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <functional>
#include <limits>
#include <filesystem>

struct User
{
    std::string username;
    std::string passwordHash;
};

std::string hashPassword(const std::string& password)
{
    std::hash<std::string> hasher;
    return std::to_string(hasher(password));
}

std::filesystem::path getUserFilePath()
{
#ifdef USER_FILE_PATH
    return std::filesystem::path(USER_FILE_PATH);
#else
    return std::filesystem::path("data") / "users.txt";
#endif
}

bool prepareUserFile(const std::filesystem::path& filename)
{
    std::filesystem::path folder = filename.parent_path();

    if (!folder.empty())
    {
        std::error_code error;
        std::filesystem::create_directories(folder, error);

        if (error)
        {
            std::cout << "Error: Could not create data folder." << std::endl;
            std::cout << "Folder: " << folder.string() << std::endl;
            return false;
        }
    }

    std::ofstream file(filename, std::ios::app);

    if (!file)
    {
        std::cout << "Error: Could not open user file." << std::endl;
        std::cout << "File path: " << filename.string() << std::endl;
        return false;
    }

    return true;
}

void displayMainMenu()
{
    std::cout << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "      User Authentication System" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "1. Register" << std::endl;
    std::cout << "2. Login" << std::endl;
    std::cout << "3. Exit" << std::endl;
    std::cout << "Please choose an option: ";
}

void displayProtectedMenu()
{
    std::cout << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "          Protected Menu" << std::endl;
    std::cout << "====================================" << std::endl;
    std::cout << "1. View Profile" << std::endl;
    std::cout << "2. Change Password" << std::endl;
    std::cout << "3. Logout" << std::endl;
    std::cout << "Please choose an option: ";
}

bool readMenuChoice(int& choice)
{
    std::cin >> choice;

    if (std::cin.fail())
    {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Invalid input. Please enter a number." << std::endl;
        return false;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return true;
}

bool usernameExists(const std::vector<User>& users, const std::string& username)
{
    for (const User& user : users)
    {
        if (user.username == username)
        {
            return true;
        }
    }

    return false;
}

bool registerUser(std::vector<User>& users)
{
    User newUser;
    std::string password;
    std::string confirmPassword;

    std::cout << "Enter username: ";
    std::getline(std::cin, newUser.username);

    if (newUser.username.empty())
    {
        std::cout << "Username cannot be empty." << std::endl;
        return false;
    }

    if (usernameExists(users, newUser.username))
    {
        std::cout << "Username already exists. Please choose another username." << std::endl;
        return false;
    }

    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    std::cout << "Confirm password: ";
    std::getline(std::cin, confirmPassword);

    if (password.empty())
    {
        std::cout << "Password cannot be empty." << std::endl;
        return false;
    }

    if (password != confirmPassword)
    {
        std::cout << "Passwords do not match." << std::endl;
        return false;
    }

    newUser.passwordHash = hashPassword(password);
    users.push_back(newUser);

    return true;
}

int validateLogin(const std::vector<User>& users,
    const std::string& username,
    const std::string& password)
{
    std::string enteredHash = hashPassword(password);

    for (int i = 0; i < static_cast<int>(users.size()); ++i)
    {
        if (users[i].username == username &&
            users[i].passwordHash == enteredHash)
        {
            return i;
        }
    }

    return -1;
}

int loginUser(const std::vector<User>& users)
{
    std::string username;
    std::string password;

    std::cout << "Enter username: ";
    std::getline(std::cin, username);

    std::cout << "Enter password: ";
    std::getline(std::cin, password);

    int userIndex = validateLogin(users, username, password);

    if (userIndex == -1)
    {
        std::cout << "Invalid username or password." << std::endl;
        return -1;
    }

    std::cout << "Login successful. Welcome, " << username << "!" << std::endl;
    return userIndex;
}

void viewProfile(const User& user)
{
    std::cout << std::endl;
    std::cout << "========== User Profile ==========" << std::endl;
    std::cout << "Username: " << user.username << std::endl;
    std::cout << "Password: Protected" << std::endl;
}

bool changePassword(std::vector<User>& users, int userIndex)
{
    std::string currentPassword;
    std::string newPassword;
    std::string confirmPassword;

    std::cout << "Enter current password: ";
    std::getline(std::cin, currentPassword);

    if (users[userIndex].passwordHash != hashPassword(currentPassword))
    {
        std::cout << "Current password is incorrect." << std::endl;
        return false;
    }

    std::cout << "Enter new password: ";
    std::getline(std::cin, newPassword);

    std::cout << "Confirm new password: ";
    std::getline(std::cin, confirmPassword);

    if (newPassword.empty())
    {
        std::cout << "New password cannot be empty." << std::endl;
        return false;
    }

    if (newPassword != confirmPassword)
    {
        std::cout << "New passwords do not match." << std::endl;
        return false;
    }

    users[userIndex].passwordHash = hashPassword(newPassword);
    return true;
}

bool saveUsersToFile(const std::vector<User>& users,
    const std::filesystem::path& filename)
{
    if (!prepareUserFile(filename))
    {
        return false;
    }

    std::ofstream file(filename);

    if (!file)
    {
        std::cout << "Error: Could not open user file for writing." << std::endl;
        std::cout << "File path: " << filename.string() << std::endl;
        return false;
    }

    for (const User& user : users)
    {
        file << user.username << "|"
            << user.passwordHash << std::endl;
    }

    file.close();

    return true;
}

bool loadUsersFromFile(std::vector<User>& users,
    const std::filesystem::path& filename)
{
    if (!prepareUserFile(filename))
    {
        return false;
    }

    std::ifstream file(filename);

    if (!file)
    {
        std::cout << "Error: Could not open user file for reading." << std::endl;
        std::cout << "File path: " << filename.string() << std::endl;
        return false;
    }

    users.clear();

    std::string line;

    while (std::getline(file, line))
    {
        std::stringstream ss(line);
        User user;

        std::getline(ss, user.username, '|');
        std::getline(ss, user.passwordHash);

        if (!user.username.empty() && !user.passwordHash.empty())
        {
            users.push_back(user);
        }
    }

    file.close();

    std::cout << "User records loaded successfully." << std::endl;
    return true;
}

void protectedArea(std::vector<User>& users,
    int userIndex,
    const std::filesystem::path& filename)
{
    int choice;

    while (true)
    {
        displayProtectedMenu();

        if (!readMenuChoice(choice))
        {
            continue;
        }

        switch (choice)
        {
        case 1:
            viewProfile(users[userIndex]);
            break;

        case 2:
            if (changePassword(users, userIndex))
            {
                if (saveUsersToFile(users, filename))
                {
                    std::cout << "Password changed successfully." << std::endl;
                }
                else
                {
                    std::cout << "Password changed in memory, but could not be saved to file." << std::endl;
                }
            }
            break;

        case 3:
            std::cout << "You have logged out." << std::endl;
            return;

        default:
            std::cout << "Invalid option. Please choose again." << std::endl;
        }
    }
}

int main()
{
    std::vector<User> users;
    std::filesystem::path filename = getUserFilePath();
    int choice;

    loadUsersFromFile(users, filename);

    while (true)
    {
        displayMainMenu();

        if (!readMenuChoice(choice))
        {
            continue;
        }

        switch (choice)
        {
        case 1:
            if (registerUser(users))
            {
                if (saveUsersToFile(users, filename))
                {
                    std::cout << "Registration successful." << std::endl;
                }
                else
                {
                    users.pop_back();
                    std::cout << "Registration failed because the user file could not be saved." << std::endl;
                }
            }
            break;

        case 2:
        {
            int userIndex = loginUser(users);

            if (userIndex != -1)
            {
                protectedArea(users, userIndex, filename);
            }

            break;
        }

        case 3:
            saveUsersToFile(users, filename);
            std::cout << "Thank you for using the User Authentication System." << std::endl;
            return 0;

        default:
            std::cout << "Invalid option. Please choose again." << std::endl;
        }
    }
}