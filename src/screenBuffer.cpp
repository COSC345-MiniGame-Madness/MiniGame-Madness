
#include "ScreenBuffer.h"


// Throws an error if the result is false
void ScreenBuffer::throwError(BOOL result, const std::string& message) const
{
    if (!result)
    {
        DWORD errorCode = GetLastError();
        std::string fullMessage = message + " Error Code: " + std::to_string(errorCode);
        throw std::runtime_error(fullMessage);
    }
}


// Write string screen buffer at a specific location
BOOL ScreenBuffer::writeToScreenBuffer(int x, int y, const std::wstring& text)
{
    // Move the cursor to the specified position
    setCursorPosition(x, y);

	// Write the text to the screen buffer
	return writeToScreenBuffer(text);
}

// Write string to screen buffer at current cursor position
BOOL ScreenBuffer::writeToScreenBuffer(const std::wstring& text)
{
	// Number of characters written
	DWORD written;

	return WriteConsoleW(
		screenHandle,  // Console screen buffer handle
		text.c_str(),  // Buffer containing the text to write
		text.length(), // Number of characters to write
		&written,      // Variable to receive the number of characters written
		nullptr        // Not using asynchronous writing
	);
}

// Constructor for the ScreenBuffer class
ScreenBuffer::ScreenBuffer()
{
    // Check if the screen buffer was created successfully
    throwError(screenHandle != INVALID_HANDLE_VALUE, "Error creating screen buffer");

    // set screen buffer size to window size
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo = getScreenBufferInfo();

    // Enable virtual terminal processing
    DWORD consoleMode;
    GetConsoleMode(screenHandle, &consoleMode);
    consoleMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(screenHandle, consoleMode);

	// Clear the screen buffer
	clearScreen();

    // Extract window size to set screen buffer size
    SMALL_RECT windowSize = ScreenBufferInfo.srWindow;
    int width = windowSize.Right - windowSize.Left + 1;
    int height = windowSize.Bottom - windowSize.Top + 1;

    setScreenSize(width, height);
}


// Destructor for the ScreenBuffer class
ScreenBuffer::~ScreenBuffer()
{
    // Close the screen buffer
    CloseHandle(screenHandle);
}


// Get the screen buffer handle
HANDLE ScreenBuffer::getScreenHandle() const
{
    return screenHandle;
}


// Is screen buffer active
bool ScreenBuffer::isActive() const
{
    HANDLE currentScreenHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    return screenHandle == currentScreenHandle;
}

// Set the screen buffer to active
void ScreenBuffer::setActive()
{
    // Set the screen buffer to active
    BOOL result = SetConsoleActiveScreenBuffer(screenHandle);

    // Check if the screen buffer was set to active successfully
    throwError(result, "Error setting screen buffer to active");
}


// Get screen buffer info
CONSOLE_SCREEN_BUFFER_INFO ScreenBuffer::getScreenBufferInfo() const
{
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo;
    BOOL result = GetConsoleScreenBufferInfo(screenHandle, &ScreenBufferInfo);

    // Check if the screen buffer info was retrieved successfully
    throwError(result, "Error getting screen buffer info");

    return ScreenBufferInfo;
}


// Clears the screen buffer
void ScreenBuffer::clearScreen()
{
    // Escape sequence to clear the screen and move cursor to the top left
    const std::wstring clearScreenSequence = L"\033[2J\033[H";

    // Write the escape sequence to the screen buffer
    BOOL result = writeToScreenBuffer(0, 0, clearScreenSequence);

    // Check if the screen buffer was cleared successfully
    throwError(result, "Error clearing screen buffer");
}


// Get the screen buffer width
int ScreenBuffer::getScreenWidth() const
{
    // Get the screen buffer info
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo = getScreenBufferInfo();

    return ScreenBufferInfo.dwSize.X;
}


// Get the screen buffer height
int ScreenBuffer::getScreenHeight() const
{
    // Get the screen buffer info
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo = getScreenBufferInfo();

    return ScreenBufferInfo.dwSize.Y;
}


// Gets the size of the screen buffer
int ScreenBuffer::getScreenSize() const
{
    // Get the screen buffer info
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo = getScreenBufferInfo();

    return ScreenBufferInfo.dwSize.X * ScreenBufferInfo.dwSize.Y;
}


// Sets the size of the screen buffer and window
void ScreenBuffer::setScreenSize(int width, int height)
{
    // Get the current screen buffer info
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo = getScreenBufferInfo();

    // Calculate the new window size
    SMALL_RECT windowSize = { 0, 0, static_cast<SHORT>(width) - 1, static_cast<SHORT>(height) - 1 };

    // If the new size is smaller, adjust the window size first
    if (width < ScreenBufferInfo.dwSize.X || height < ScreenBufferInfo.dwSize.Y)
    {
        BOOL result1 = SetConsoleWindowInfo(screenHandle, TRUE, &windowSize);
        throwError(result1, "Error setting console window size");
    }

    // Set the screen buffer size to the specified size
    COORD size = { static_cast<SHORT>(width), static_cast<SHORT>(height)};
    BOOL result2 = SetConsoleScreenBufferSize(screenHandle, size);
    throwError(result2, "Error setting screen buffer size");

    // If the new size is larger, adjust the window size after setting the buffer size
    if (width > ScreenBufferInfo.dwSize.X || height > ScreenBufferInfo.dwSize.Y)
    {
        BOOL result3 = SetConsoleWindowInfo(screenHandle, TRUE, &windowSize);
        throwError(result3, "Error setting console window size");
    }
}


// Get the screen text and background colours
std::pair<WORD, WORD> ScreenBuffer::getScreenColours(int x, int y, int length) const
{
    // Start coordiante based on x and y
    COORD position = {static_cast<SHORT>(x), static_cast<SHORT>(y)};

    // Declare the attributes vector
    std::vector<WORD> attributes(length);

    // number of attributes read
    DWORD read;

    // Read the attributes from the console buffer at the specified position
    BOOL result = ReadConsoleOutputAttribute(
        screenHandle,      // Console screen buffer handle
        attributes.data(), // Buffer to store the attributes
        length,            // Number of attributes to read
        position,          // Coordinates where to start reading
        &read              // Variable to receive the number of attributes read
    );

    // Check if the attributes were read successfully
    throwError(result, "Error getting screen colours");

    return std::make_pair(attributes[0] & 0x0F, attributes[0] >> 4);
}


// Set the screen text colour and background colour
std::wstring ScreenBuffer::setTextColours(const std::wstring& text, WORD textColour, WORD backgroundColour)
{
    // String to hold the VT sequence
    std::wstring vtSequence = L"\033[";

    // Apply text color if specified

    if (textColour != NO_COLOUR)
    {
        vtSequence += std::to_wstring(30 + textColour);
    }

    // Apply background color if specified
    if (backgroundColour != NO_COLOUR)
    {
        if (textColour != NO_COLOUR)
        {
            vtSequence += L";";
        }
        vtSequence += std::to_wstring(40 + backgroundColour);
    }

    vtSequence += L"m" + text + L"\033[0m"; // Append text and reset sequence

    return vtSequence;
}


// Change curser visablitly
void ScreenBuffer::setCursorVisibility(bool isVisible)
{
	BOOL result;

	if (isVisible)
	{
		result = writeToScreenBuffer(L"\033[?25h");
	}
	else
	{
		result = writeToScreenBuffer(L"\033[?25l");
	}

    // Check if the cursor visibility was set successfully
    throwError(result, "Error setting cursor visibility");
}


// Moves the curser to the specified location
void ScreenBuffer::setCursorPosition(int x, int y)
{
    COORD coord;
    coord.X = static_cast<SHORT>(x);
    coord.Y = static_cast<SHORT>(y);
    BOOL result = SetConsoleCursorPosition(screenHandle, coord);

    // Check if the cursor position was set successfully
    throwError(result, "Error setting cursor position");
}


// Gets the current location of the curse
std::pair<int, int> ScreenBuffer::getCursorPosition() const
{
    // Get the screen buffer info
    CONSOLE_SCREEN_BUFFER_INFO ScreenBufferInfo = getScreenBufferInfo();

    return std::make_pair(ScreenBufferInfo.dwCursorPosition.X, ScreenBufferInfo.dwCursorPosition.Y);
}


// Get text from a section of the screen buffer
std::wstring ScreenBuffer::readScreenText(int x, int y, int length) const
{

    // Start coordiante based on x and y
    COORD position = {static_cast<SHORT>(x), static_cast<SHORT>(y)};

    // Create a buffer to store the text
	std::wstring text(length, ' ');

    // Number of characters read
    DWORD read;

    // Read the text from the console buffer at the specified position
    BOOL result = ReadConsoleOutputCharacterW(
        screenHandle, // Console screen buffer handle
        &text[0],     // Buffer to store the text
        length,       // Number of characters to read
        position,     // Coordinates where to start reading
        &read         // Variable to receive the number of characters read
    );
    // Check if the text was read successfully
    throwError(result, "Error getting screen text");

    return text;
}


// Get all of the text from the screen buffer
std::wstring ScreenBuffer::readAllScreenText() const
{
    // Get the text from the screen buffer
    return readScreenText(0, 0, getScreenSize());
}


// Writes text to the screen at a specific location
void ScreenBuffer::writeToScreen(int x, int y, const std::wstring& text)
{
    // Write the text to the screen
    BOOL result = writeToScreenBuffer(x, y, text);

    // Check if the text was written successfully
    throwError(result, "Error writing to screen");
}


// Writes text to the screen at a specific location with a specific colour
void ScreenBuffer::writeToScreen(int x, int y, const std::wstring& text, WORD textColour, WORD backgroundColour)
{
    // write the text to the screen
    writeToScreen(x, y, setTextColours(text, textColour, backgroundColour));
}

