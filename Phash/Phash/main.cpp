//#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>
//opencv_ts300.lib
//opencv_ts300d.lib
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>

using namespace std;

// Рассчет хэша изображения
__int64 calcImageHash(IplImage* image, bool show_results = false);
// Рассчитываем расстояние Хэмминга между хэшами
__int64 calcHammingDistance(__int64 x, __int64 y);

int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "russian");
	IplImage *object = 0, *image = 0;

	char obj_name[] = "k32.jpg";
	char img_name[] = "k31.jpg";

	// Имя объекта задаётся первым параметром
	char* object_filename = argc >= 2 ? argv[1] : obj_name;
	// Имя картинки задаётся вторым параметром
	char* image_filename = argc >= 3 ? argv[2] : img_name;

	// Получаем изображения
	object = cvLoadImage(object_filename, 1);
	image = cvLoadImage(image_filename, 1);

	printf("[i] Объект для сравнения: %s\n", object_filename);
	printf("[i] Изображение: %s\n", image_filename);
	if (!object) {
		printf("[!] Ошибка, объект не загружен: %s\n", object_filename);
		return -1;
	}
	if (!image) {
		printf("[!] Ошибка, изображение не загружено: %s\n", image_filename);
		return -1;
	}

	// Показываем изображения
	cvNamedWindow("Объект для сравнения");
	cvShowImage("Объект для сравнения", object);
	cvNamedWindow("Изображение");
	cvShowImage("Изображение", image);

	// Строим хэш
	__int64 hashO = calcImageHash(object, true);

	__int64 hashI = calcImageHash(image, false);

	// Рассчитываем расстояние Хэмминга между хэшами
	__int64 dist = calcHammingDistance(hashO, hashI);

	printf("[i] Расстояние Хэмминга: %I64d \n", dist);

	// Ожидание действия
	cvWaitKey(0);

	// Освободить ресурсы
	cvReleaseImage(&object);
	cvReleaseImage(&image);

	// Удалить окна
	cvDestroyAllWindows();
	return 0;
}

// Рассчет хэша изображения
__int64 calcImageHash(IplImage* src, bool show_results)
{
	if (!src) {
		return 0;
	}

	IplImage *res = 0, *gray = 0, *bin = 0;

	res = cvCreateImage(cvSize(8, 8), src->depth, src->nChannels);
	gray = cvCreateImage(cvSize(8, 8), IPL_DEPTH_8U, 1);
	bin = cvCreateImage(cvSize(8, 8), IPL_DEPTH_8U, 1);

	// Уменьшаем изображение
	cvResize(src, res);
	// Переводим изображение в градации серого
	cvCvtColor(res, gray, CV_BGR2GRAY);
	// Вычисляем среднее
	CvScalar average = cvAvg(gray);
	printf("[i] Среднее: %.2f \n", average.val[0]);
	// Получим бинарное изображение относительно среднего
	// Для этого пользуемся пороговым преобразованием
	cvThreshold(gray, bin, average.val[0], 255, CV_THRESH_BINARY);

	// Строим хэш
	__int64 hash = 0;

	int i = 0;
	//Биты слева направо, сверху вниз
	// Проходим по всем пикселям изображения
	for (int y = 0; y < bin->height; y++) {
		uchar* ptr = (uchar*)(bin->imageData + y * bin->widthStep);
		for (int x = 0; x < bin->width; x++) {
			// 1 канал
			if (ptr[x]) {
				// Переводим полученные 64 значений 1 и 0 картинки в одно 64-битное значение хэша
				hash |= 1i64 << i;
			}
			i++;
		}
	}

	printf("[i] Перцептивный хэш: %I64X \n", hash);

	if (show_results) {
		// Увеличенные картинки для отображения результатов
		IplImage* dst3 = cvCreateImage(cvSize(128, 128), IPL_DEPTH_8U, 3);
		IplImage* dst1 = cvCreateImage(cvSize(128, 128), IPL_DEPTH_8U, 1);

		// Показать картинки
		cvNamedWindow("64");
		cvResize(res, dst3, CV_INTER_NN);
		cvShowImage("64", dst3);
		cvNamedWindow("gray");
		cvResize(gray, dst1, CV_INTER_NN);
		cvShowImage("gray", dst1);
		cvNamedWindow("bin");
		cvResize(bin, dst1, CV_INTER_NN);
		cvShowImage("bin", dst1);

		cvReleaseImage(&dst3);
		cvReleaseImage(&dst1);
	}

	// Освобоить ресурсы
	cvReleaseImage(&res);
	cvReleaseImage(&gray);
	cvReleaseImage(&bin);

	return hash;
}

// Рассчитываем расстояние Хэмминга между хэшами

__int64 calcHammingDistance(__int64 x, __int64 y)
{
	__int64 dist = 0, val = x ^ y;

	// Подсчитать количество установленных бит
	while (val)
	{
		++dist;
		val &= val - 1;
	}

	if (dist <= 10) 
	{
		cout << "|||Вероятнее всего, изображения одинаковые, с некоторыми небольшими различиями|||\n";
	}
	else if (dist > 10 && dist <= 15) 
	{
		cout << "|||Вероятно, изображения имеют средние различия, либо они разные|||\n";
	}
	else 
	{
		cout << "|||Вероятнее всего, изображения разные, либо имеют большие различия|||\n";
	}

	return dist;
}