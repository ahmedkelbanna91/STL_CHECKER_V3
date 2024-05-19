#include <conio.h> // Add this header for _kbhit() function
#include <future>  // Add this header for std::async()
#include <chrono>
#include <thread>
#include <iomanip>
#include <atomic>
#include <filesystem>
#include <iostream>
#include <limits>
#include <fstream>
#include <windows.h>

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/IO/STL.h>


typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Surface_mesh<Point> Mesh;
typedef boost::graph_traits<Mesh>::face_descriptor face_descriptor;
int files_total = 0;
int files_with_multiple_shells = 0;
int files_with_one_shell = 0;

bool read_STL(const std::string& filepath, Mesh& mesh) {
    std::ifstream input(filepath, std::ios::binary);
    if (!input) {
        std::cerr << "Error: Cannot open the STL file " << std::filesystem::path(filepath).filename() << std::endl;
        return false;
    }
    CGAL::IO::set_binary_mode(input);
    if (!CGAL::IO::read_STL(input, mesh)) {
        std::cerr << "Error: Cannot read the STL file " << std::filesystem::path(filepath).filename() << std::endl;
        return false;
    }
    return true;
}

int main(int argc, char* argv[]) {

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return 1;
    }
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) {
        return 1;
    }
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) {
        return 1;
    }

    std::filesystem::path current_path = std::filesystem::current_path();
    std::filesystem::path Please_Repair_path = current_path / "Please_Repair";

    std::cout << "\n \x1B[93m============================'Created by Banna'===============================\x1B[0m" << std::endl;
    std::cout << " \x1B[93m============================='STL CHECKER V3'================================\x1B[0m\n\n" << std::endl;

    // Record the start time
    auto start_time_total = std::chrono::steady_clock::now();

    for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
        if (entry.path().extension() == ".stl" || entry.path().extension() == ".STL") {
            Mesh mesh;

            std::string file_path = entry.path().string();
           
            read_STL(file_path.c_str(), mesh);

            if (mesh.is_empty()) {
                std::cerr << entry.path().filename().string() << " - Not a valid Mesh." << std::endl;
            }


            // Create a property map for connected component indices
            std::vector<std::size_t> component_map(num_faces(mesh));
            auto fccmap = boost::make_iterator_property_map(component_map.begin(), get(boost::face_index, mesh));

            // Calculate the number of connected components
            std::size_t number_of_shells = CGAL::Polygon_mesh_processing::connected_components(mesh, fccmap);


            ++files_total;
            if (number_of_shells > 1) {
                ++files_with_multiple_shells;

                std::cout << std::left << "  " << std::setw(4) << files_total << "- \x1B[91m" << std::setw(45) << entry.path().filename().string() << "\x1B[0m"
                    << "Shells: \x1B[91m" << std::setw(4) << number_of_shells << "\x1B[0m- Please_Repair";

                /*if (!std::filesystem::exists(Please_Repair_path)) {
                    std::filesystem::create_directory(Please_Repair_path);
                }
                std::filesystem::path destination_path = Please_Repair_path / entry.path().filename();
                std::filesystem::rename(entry.path(), destination_path);*/
            }
            else {
                ++files_with_one_shell;
                std::cout << std::left << "  " << std::setw(4) << files_total << "- \x1B[92m" << std::setw(45) << entry.path().filename().string() << "\x1B[0m"
                    << "\x1B[0mShells: \x1B[92m" << number_of_shells << "\x1B[0m";
            }
            std::cout << "\n" << std::endl;
        }
    }
    

    // Compute the elapsed time
    auto end_time = std::chrono::steady_clock::now();
    auto elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time_total);
    //int elapsed_minutes = elapsed_time.count() / 60;
    int elapsed_seconds = elapsed_time.count() % 60;

    std::cout << "\n \x1B[93m================================='Report'====================================\x1B[0m\n" << std::endl;
    if (files_with_multiple_shells > 0) {
        std::cout << " \x1B[91mSTL Need repair:     " << files_with_multiple_shells << " / " << std::setw(10) << files_total
            << "\x1B[0mMoved to 'Please_Repair' folder" << std::endl;
    }
    std::cout << " \x1B[92mGood STL files:      " << files_with_one_shell << " / " << files_total << "\x1B[0m" << std::endl;

    //std::cout << "\n \x1B[93mElapsed time:        " << elapsed_minutes << " minutes and " << elapsed_seconds << " seconds\x1B[0m\n" << std::endl;
    std::cout << "\n \x1B[93mElapsed time:        " << elapsed_seconds << " seconds\x1B[0m\n" << std::endl;

    // Exit program
    int Delay = 10;
    int remaining_seconds = Delay;
    std::cout << "\n" << std::endl;

    auto input_checker = std::async(std::launch::async, [&]() {
        while (remaining_seconds > 0) {
            if (_kbhit()) {
                _getch(); // Read the pressed key and discard it
                return true;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Sleep for a short duration
        }
        });

    while (remaining_seconds > 0 && input_checker.wait_for(std::chrono::seconds(1)) == std::future_status::timeout) {
        remaining_seconds = Delay - std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - end_time).count();
        std::cout << "\r\x1B[93mPress ENTER to EXIT or wait " << remaining_seconds << " seconds...\x1B[0m" << std::flush;
    }

    return EXIT_SUCCESS;
}