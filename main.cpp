#include<opencv2/imgcodecs.hpp>
#include<opencv2/highgui.hpp>
#include <iostream>

// we're NOT "using namespace std;" here, to avoid collisions between the beta variable and std::beta in c++17
using std::cin;
using std::cout;
using std::endl;
using namespace cv;
int main( int argc, char** argv )
{
    CommandLineParser parser( argc, argv, "{@input | ../data/lena.jpg | input image}" );
    Mat image = imread( parser.get<String>( "@input" ) );
    if( image.empty() )
    {
        cout << "Could not open or find the image!\n" << endl;
        cout << "Usage: " << argv[0] << " <Input image>" << endl;
        return -1;
    }
    Mat new_image = Mat::zeros( image.size(), image.type() );
    Mat other_image = Mat::zeros(image.size() , image.type());
    // essas duas mat serão para a LUT
    Mat table = Mat::zeros(1,256 , CV_8U);
    Mat table2 = Mat::zeros(1,256,CV_8U);
    double alpha = 1.0; /*< Simple contrast control */
    int beta = 0;       /*< Simple brightness control */
    double gama = 1;
    int key;
    do{
        cout << "calculando nova imagem\n";
        // isso é para calcular a LUT
        uchar *ptr = table.ptr();
        uchar *ptr2 = table2.ptr();
        for(int i = 0 ; i < 256 ; i++){
            // sem esse saturate_cast ele não arredonda direito, o valor sem isso da overflow e fica 0 em vez de ficar 255
            ptr[i] = saturate_cast<uchar>(alpha * i + beta);
            ptr2[i] = saturate_cast<uchar>(std::pow(i/255.0 , gama) * 255.0);
        }
        // a table contém um algoritmo simpes para aumentar o brilho das imagens, somando uma constante para todos
        // os valores de cor da imagem
        // a table2 utiliza o método gamma correction que acentua o brilho de valores mais baixos do que valores mais altos
        // o oposto também é valido
        cout << "\ngama: " << gama << "\nbeta: " << beta;
        cv::LUT(image , table2 , other_image);
        cv::LUT(image , table , new_image);
        imshow("Original Image", image);
        imshow("gama" , other_image);
        imshow("New Image", new_image);
        cout << "calculou nova imagem\n";
        key = waitKey();
        if(key == 'j'){
            if(beta > -255) beta--;
            if(gama < 50.0) gama+=0.02;
        }else if(key == 'k'){
            if(beta < 255) beta++;
            if(gama > 0.0) gama-=0.02;
        }
    }while(key != 27);
    return 0;
}
