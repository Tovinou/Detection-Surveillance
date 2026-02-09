#ifndef HTML_PAGE_H
#define HTML_PAGE_H

#include <Arduino.h>
#include <WString.h>

// Forward declarations or include headers
// Using relative paths to ensure they are found regardless of include path settings
#include "../../include/state.h"
#include "../../include/config.h"

class HtmlPage {
public:
    // Set external dependencies
    static void setSystemState(SystemState* state);
    static void setConfig(AppConfig* cfg);
    
    // Page Generators
    static String generateDashboard();
    static String generateAPIResponse();
    static String generateConfigPage();
    static String generateHistoryPage();
    static String generateErrorPage(const String& message);
    
private:
    // Helper methods for UI components
    static String getCSS();
    static String getHeader();
    static String getFooter();
    static String formatTimestamp(unsigned long timestamp);
    
    // Dependencies
    static SystemState* systemState;
    static AppConfig* config;
};

#endif
