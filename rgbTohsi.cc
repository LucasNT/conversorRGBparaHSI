#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include <iostream>
#include <cstdint>
#include <algorithm>
#include <cmath>
#include <atomic>


double cosDegree ( double degree ){
    return std::cos(degree * 3.1415926535897 / 180);
}

cv::Mat RGBToHSI( cv::Mat &img ){
    cv::Mat hsi(img.size() , CV_64FC3);
    img.forEach<cv::Vec3b>( [&] (cv::Vec3b &pi , const int *position) -> void {
            auto &vecHSI = hsi.at<cv::Vec3d>(position[0] , position[1]);
            double r,g,b;
            b = pi[0]/255.0;
            g = pi[1]/255.0;
            r = pi[2]/255.0;
            vecHSI[2] = (r+g+b) / 3.0;
            if(vecHSI[2])
                vecHSI[1] = 1-( std::min(r , std::min(g , b)) /vecHSI[2] );
            else
                vecHSI[1] = 0;
            vecHSI[1] = vecHSI[1] < 1e-5 ? 0 : vecHSI[1] > 0.99999 ? 1 : vecHSI[1];
            double rg = r - g;
            double rb = r - b;
            vecHSI[0] = (std::acos((rg + rb)/(std::sqrt(rg*rg + rb * (g - b))*2 +0.000000000001)) * 180)/3.1415926535897;
            if(pi[0] > pi[1])
                vecHSI[0] = 360 - vecHSI[0];
    });
    /*
    auto aux = hsi.at<cv::Vec3d>(0,0);
    std::cout << "(0,0) -> " << aux[0] << " " << aux[1] << " " << aux[2] << "\n";

    aux = hsi.at<cv::Vec3d>(1,0);
    std::cout << "(1,0) -> " << aux[0] << " " << aux[1] << " " << aux[2] << "\n";

    aux = hsi.at<cv::Vec3d>(2,0);
    std::cout << "(2,0) -> " << aux[0] << " " << aux[1] << " " << aux[2] << "\n";

    aux = hsi.at<cv::Vec3d>(3,0);
    std::cout << "(3,0) -> " << aux[0] << " " << aux[1] << " " << aux[2] << "\n";
    std::cout << "\n";
    */

    return hsi;
}

cv::Mat HSIToRGB(cv::Mat &img){
    cv::Mat rgb(img.size() , CV_8UC3);
    img.forEach<cv::Vec3d>( [&] (cv::Vec3d &pixel , const int *position) -> void {
        cv::Vec3b &vec = rgb.at<cv::Vec3b>(position[0] , position[1]);
        if(pixel[0] <= 120){
            vec[0] = pixel[2] * (1 - pixel[1]) * 255;
            vec[2] = pixel[2] * ( 1 + ((pixel[1] * cosDegree(pixel[0])) / cosDegree(60 - pixel[0]))) * 255;
            vec[1] = 3 * pixel[2] * 255 - vec[2] - vec[0];
        }else if(pixel[0] <= 240){
            vec[2] = pixel[2] * (1 - pixel[1]) * 255;
            vec[1] = pixel[2] * ( 1 + ((pixel[1] * cosDegree(pixel[0] - 120)) / cosDegree(180 - pixel[0])))*255;
            vec[0] = 3 * pixel[2] * 255 - vec[2] - vec[1];
        }else {
            vec[1] = pixel[2] * (1 - pixel[1]) * 255;
            vec[0] = pixel[2] * ( 1 + ((pixel[1] * cosDegree(pixel[0] - 240)) / cosDegree(300 - pixel[0]))) * 255;
            vec[2] = 3 * pixel[2] * 255 - vec[1] - vec[0];
        }
    });
    /*
    auto aux = rgb.at<cv::Vec3b>(0,0);
    std::cout << "(0,0) -> " << (unsigned int)aux[0] << " " << (unsigned int)aux[1] << " " << (unsigned int)aux[2] << "\n";

    aux = rgb.at<cv::Vec3b>(1,0);
    std::cout << "(1,0) -> " << (unsigned int)aux[0] << " " << (unsigned int)aux[1] << " " << (unsigned int)aux[2] << "\n";

    aux = rgb.at<cv::Vec3b>(2,0);
    std::cout << "(2,0) -> " << (unsigned int)aux[0] << " " << (unsigned int)aux[1] << " " << (unsigned int)aux[2] << "\n";

    aux = rgb.at<cv::Vec3b>(3,0);
    std::cout << "(3,0) -> " << (unsigned int)aux[0] << " " << (unsigned int)aux[1] << " " << (unsigned int)aux[2] << "\n";
    std::cout << "\n";
    */
    return rgb;
}

template< class T>
void compareTwoMat(cv::Mat &img1 , cv::Mat img2){
    std::atomic<int> qtdErro(0);
    unsigned int difSum = 0;
    uint32_t sumElemtDiff = 0;
    img1.forEach<T>([&] (T &ponto , const int *position) -> void {
        auto aux =  img2.at<T>(position[0] , position[1]);
        if(aux != ponto){
            auto resu = aux - ponto;
            int32_t soma1 = 0;
            int32_t soma2 = 0;
            for(int i = 0 ; i < resu.rows ; i++){
                difSum += resu[i] >= 0 ? resu[i] : -resu[i];
                soma1 += aux[i];
                soma2 += ponto[i];
            }
            sumElemtDiff += (soma2 != soma1) ;
            qtdErro++;
        }
    });
    uint32_t total = (img2.rows * img2.cols);
    std::cout << qtdErro << " cores erradas de " << total << "\n";
    std::cout << (qtdErro/((double) total) * 100) << "%\n";
    std::cout << "Média do Erro: " << (difSum/(double)qtdErro) << "\n";
    std::cout << difSum << "\n";
    std::cout << sumElemtDiff << "\n";
}



int main (int argc , char* argv[]){
    cv::CommandLineParser parser(argc , argv , "{@input | ./a.png | input image}");
    cv::Mat image = cv::imread(parser.get<std::string>("@input"));

    if( image.empty() ){
        std::cout << "Imagem não encontrada\n";
        std::cout << "Modo de Uso:\n" << argv[0] << "<ImagemDeEntrada>" << std::endl;
        return -1;
    }

    /*
    auto aux = image.at<cv::Vec3b>(0,0);
    std::cout << "(0,0) -> " << (unsigned int)aux[0] << " " << (unsigned int)aux[1] << " " << (unsigned int)aux[2] << "\n";

    aux = image.at<cv::Vec3b>(1,0);
    std::cout << "(1,0) -> " << (unsigned int)aux[0] << " " << (unsigned int)aux[1] << " " << (unsigned int)aux[2] << "\n";

    aux = image.at<cv::Vec3b>(2,0);
    std::cout << "(2,0) -> " << (unsigned int)aux[0] << " " << (unsigned int)aux[1] << " " << (unsigned int)aux[2] << "\n";

    aux = image.at<cv::Vec3b>(3,0);
    std::cout << "(3,0) -> " << (unsigned int)aux[0] << " " << (unsigned int)aux[1] << " " << (unsigned int)aux[2] << "\n";
    std::cout << "\n";
    */

    cv::Mat hsi = RGBToHSI(image);
    cv::Mat rgb = HSIToRGB(hsi);
    compareTwoMat<cv::Vec3b>(rgb , image);
    cv::waitKey();

    return 0;
}
