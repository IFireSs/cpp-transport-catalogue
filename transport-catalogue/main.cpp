#include <iostream>
#include "json_reader.h"
using namespace std;

int main() {
    transport::core::TransportCatalogue catalogue; 
    ParseJson(LoadJSON(cin), catalogue); 

}
