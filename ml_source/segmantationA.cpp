
//#include	<direct.h>				//windows 1
//#include	<opencv2/opencv.hpp>	//windows 1
#include <sys/stat.h>				//linux 1
#include <unistd.h>				//linux 1
#include <opencv2/opencv.hpp>		//linux 1

using namespace cv;

//////////////extern "C" { //windows 2
extern "C" {			//linux 2 

	#define DATA_N  (100)
	#define CALC_PITCH(x,y)			(((x)*(y))%4==0) ? (x)*(y): ((((x)*(y))/4)*4+4);

	//windows 3
	//#define   STRCPY(a,b)			strcpy_s(a,b)
	//#define   SPRINTF               sprintf_s
	//#define	  FOPEN(fp,file,mod)	fopen_s(&fp,file,mod)
	//#define	  SSCANF				sscanf_s
	//#define	SpRINTF7(a,b,c,d,e,f,g)	sprintf_s(a,b,c,d,e,f,g)
	//#define	SpRINTF6(a,b,c,d,e,f)	sprintf_s(a,b,c,d,e,f)
	//#define PL()					{printf("%d:\n",__LINE__);}
	//linux 3
	#define STRCPY(a,b)			strcpy(a,b)
	#define SPRINTF               sprintf
	#define FOPEN(file,mod)	fopen(file,mod)
	#define SSCANF				sscanf
	#define SpRINTF7(a,b,c,d,e,f,g)	sprintf(a,c,d,e,f,g)
	#define SpRINTF6(a,b,c,d,e,f)		sprintf(a,c,d,e,f)
	#define PL()					{}

	static unsigned char* Buf0;

	int load_image(const char* file, int* width, int* height, int* channel, int* depth);
	int add_num_to_file(char* outfile1, char* outfile, const int cha);
	int add_char_to_file(char* outfile1, char* outfile, const char* cha);
	int calc_on_label_img1ch(unsigned long* label1, unsigned char* buf, int key, int value1, int width, int height, int* toku, int label1_n);
	int check_param(unsigned long* label0, unsigned char* buf2, int width, int height, int* toku, int label1_n, int* keys, int* mins, int* maxs, int para_n);
	int change_extension(char* outfile1, char* outfile, const char* ext);
	int check_file_exist(const char* file, int* flag);
	int get_file_name_and_dir(char* full_path, char* file_name, char* file_dir);
	int merge(unsigned short* buf3, unsigned char* buf2, unsigned long* label0, int* toku, int width, int height);
	int read_src_param(char* parafile, int* posi_nega, int* rate, int* step, int* evkey, int* evflag, int* keys, int* mins, int* maxs, int* para_n);
	int report_toku30(char* file, int* toku, int label1_n);
	int Src_label2L(unsigned long* label1, unsigned char* buf74, int width, int height, int* label1_n);
	int save_image(char* file0, unsigned char* buf, int width, int height, int channel, int depth);
	int get_file_extension(char* file_name, char* file_extension, char* file_plain_name);

	int main(int argc, char* argv[])
	{
		char app[512], infile[512], outfile[512], name[512], dir[512], work[512], outfile0[512];
		char parafile[512];
		int para[10], m, n;
		int seg(char* infile, char* outfile, char* parafile, char* work);

		if (argc >= 1) STRCPY(app, argv[0]);
		if (argc >= 2) STRCPY(infile, argv[1]);
		if (argc >= 3) STRCPY(outfile0, argv[2]);
		if (argc >= 4) STRCPY(parafile, argv[3]);

		//out-path取得
		get_file_name_and_dir(outfile0, name, dir);

		//out-pathの直下に work=temp作成。（デバッグ用のファイル等を保存)
		SPRINTF(work, "%s/temp", dir);

		//_mkdir(work);//windows 4
		mkdir(work, 0777); //linux 4

		//outfileの拡張子を変更(jpg)
		change_extension(outfile, outfile0, ".jpg");

		//デバッグ用の表示(消すな)
		printf("infile   =(%s)\n", infile);
		printf("outfile  =(%s)\n", outfile);
		printf("parafile =(%s)\n", parafile);
		printf("work     =(%s)\n", work);

		seg(infile, outfile, parafile, work);

		return(0);
	}
	int seg(char* infile, char* outfile, char* parafile, char* work)
	{
		char file[512], file1[512], file2[512];
		int width, height, channel, depth;
		int src_flag, src_rate, src_step, src_evkey, src_evtype, nof_keys, src_ume;
		int keys[30], mins[30], maxs[30];
		int i, j, pitch1, pitch3;
		unsigned char* buf, * buf2, * buf2a;
		unsigned short* bufm;
		unsigned long* label1;
		int label1_n, th, * toku, * toku2;

		int Src_label3L(unsigned long* label1, unsigned char* buf74, int width, int height, int* label1_n);
		int save_label2(char* file1, char* file2, int* toku, int label1n, unsigned long* buf, int width, int height, int channel);
		int save_image_down2(char* file, unsigned short* bufm, int width, int height);
		int img_choose_white(unsigned char* buf1, int min, int width, int height);

		//画像
		load_image(infile, &width, &height, &channel, &depth);
		printf("w=%d h=%d ch=%d d=%d\n", width, height, channel, depth);

		//add_num_to_file(file,outfile,100);
		add_num_to_file(file, outfile, 100);
		save_image(file, Buf0, width, height, channel, 1);

		//パラメータファイル読み込み
		read_src_param(parafile, &src_flag, &src_rate, &src_step, &src_evkey, &src_evtype, keys, mins, maxs, &nof_keys); //パラメータ読み込み

		printf("%d %d %d %d %d %d\n", src_flag, src_rate, src_step, src_evkey, src_evtype, nof_keys);
		for (i = 0; i < nof_keys; i++)
			printf("key=%d (%d,%d)\n", keys[i], mins[i], maxs[i]);
		//src-flag：   ポジかネガか(0:nega,1:posi)
		//src_rate：   縮小率、今回は使っていない。
		//src_step：   ２値化の刻みの間隔(計算時間⇔精度）
		//src_evkey：  評価関数に用いる特徴量
		//src_evtype： 評価方法(高い・低い・値に近い)
		//key,min,max：領域選別の為の、特徴量とその上限、下限
		//nof_keys：　 特徴量の数

		src_ume = 0;
		if (src_flag >= 2)
		{
			printf("src_flag=%d \n", src_flag);
			src_ume = src_flag - (src_flag % 2);
			src_flag = (src_flag % 2);
			printf("src_flag=%d src_ume=%d\n", src_flag, src_ume);
		}
		//printf("src_flag=%d src_ume=%d\n", src_flag, src_ume);
		//getchar();

		pitch1 = CALC_PITCH(width, 1);
		pitch3 = CALC_PITCH(width, 3);

		buf = (unsigned char*)calloc(height * pitch1, 1);//グレースケール化した元画像
		buf2 = (unsigned char*)calloc(height * pitch1, 1);//加工画像
		buf2a = (unsigned char*)calloc(height * pitch1, 1);//加工画像
		bufm = (unsigned short*)calloc(2, pitch1 * height);

		long long sum[4];
		int flag[4];//各色が全部0か255かのフラグ
		int k, nof_flag, ss, max0;

		//bufは抽出対象画像
		if (channel != 1)//1色でない場合
		{
			//3,4色中1色グレーかどうかの判定
			sum[0] = sum[1] = sum[2] = sum[3] = 0;
			flag[0] = flag[1] = flag[2] = flag[3] = 0;//各色のチャネルが有効かのフラグ
			for (k = 0; k < channel; k++)
			{
				for (j = 0; (j < height) && (flag[k] == 0); j++)
					for (i = 0; i < width; i++)
					{
						sum[k] += Buf0[j * pitch3 + i * channel + k];
						//if ((j * width + i)<200) //確認用
						//{printf("k=%d n=%d sum=%d\n", k, (j* width + i), sum[k]);
						//getchar(); }
						if ((sum[k] != 0) && (sum[k] != (j * width + i) * 255))//全部が0か255でない
						{
							flag[k] = 1; break;
						}
					}
			}
			nof_flag = 0;
			for (k = 0; k < channel; k++)
				if (flag[k] != 0) nof_flag++;//有効なチャネル数

				//printf("nof_flag=%d channel=%d \n", nof_flag, channel);
				//getchar();

				//有効なチャンネルのみの平均とする
			if (nof_flag != 0)
				for (j = 0; j < height; j++)
					for (i = 0; i < width; i++)
					{
						ss = 0;
						for (k = 0; k < channel; k++)
							ss += Buf0[j * pitch3 + i * channel + k] * flag[k];
						buf[j * pitch1 + i] = ss / nof_flag;
					}
		}
		else
			memcpy(buf, Buf0, pitch1 * height);

		//グレースケール化の結果確認
		add_num_to_file(file, outfile, 101);
		save_image(file, buf, width, height, 1, 1);

		if (src_flag == 0)//反転(暗い方をサーチする場合)0:nega 1:posi
			for (i = 0; i < height * pitch1; i++) buf[i] = 255 - buf[i];

		//グレースケール確認画像(反転有無込み)
		//add_num_to_file(file,outfile,102);
		//save_image(file, buf,width,height,1,1) ;

//最大値で規格化
		max0 = 0;//最大値
		for (i = 0; i < height * pitch1; i++)
			if (buf[i] > max0) max0 = buf[i];

		if (max0 != 0)//規格化
			for (i = 0; i < height * pitch1; i++)
				buf[i] = buf[i] * 255 / max0;

		//グレースケール確認画像(反転有無込み)
		add_num_to_file(file, outfile, 103);
		save_image(file, buf, width, height, 1, 1);

		label1 = (unsigned long*)calloc(width * height, sizeof(unsigned long));

		int debug_flag0 = 0;
		//デバッグ時に途中経過を見たければdebug_flag0を1にする。
		int count, count_old;

		//メインループ
		count = 0;
		for (th = 255; th >= 0; th -= src_step)//領域を徐々に広げる
		{
			//2値化(buf2<-buf)
			count_old = count;
			count = 0;
			for (i = 0; i < height * pitch1; i++)
			{
				if (buf[i] >= th) { buf2[i] = 255; count++; }
				else            buf2[i] = 0;
			}
			if (count == count_old) continue;//前の二値化と同じならやらない。
			if (count == height * pitch1) continue;
			if (count == 0) continue;
			printf("th=%d count=%d %d\n", th, count, height * pitch1);//現在の閾値

			if (src_ume != 0)//埋め込みをおこなう場合。
			{
				memcpy(buf2a, buf2, pitch1 * height);
				//SPRINTF(file, "%s\\_91.jpg", work);
				//save_image(file, buf2a, width, height, 1, 1);
				for (i = 0; i < pitch1 * height; i++) buf2a[i] = 255 - buf2a[i];//反転
							//SPRINTF(file, "%s\\_92.jpg", work);
							//save_image(file, buf2a, width, height, 1, 1);
				img_choose_white(buf2a, src_ume, width, height);
				//SPRINTF(file, "%s\\_93.jpg", work);
				//save_image(file, buf2a, width, height, 1, 1);
				for (i = 0; i < pitch1 * height; i++) buf2[i] = buf2[i] + buf2a[i];//領域合成
							//SPRINTF(file, "%s\\_94.jpg", work);
							//save_image(file, buf2a, width, height, 1, 1);
				//getchar();
			}

			Src_label3L(label1, buf2, width, height, &label1_n);//1

			toku = (int*)calloc(4, (label1_n + 1) * DATA_N);

			//ラベリング結果の表示
			if (debug_flag0 != 0)
			{
				add_char_to_file(file1, outfile, "label1");
				SPRINTF(file1, "%s/_label1.jpg", work);
				add_char_to_file(file2, outfile, "label2");
				SPRINTF(file2, "%s/_label3.jpg", work);
				save_label2(file1, file2, toku, label1_n, label1, width, height, channel);
			}

			calc_on_label_img1ch(label1, buf, src_evkey, src_evtype, width, height, toku, label1_n);

			if (debug_flag0 != 0)
			{
				SPRINTF(file, "%s/_4_%3d.csv", work, th);
				report_toku30(file, toku, label1_n);
			}

			check_param(label1, buf2, width, height, toku, label1_n, keys, mins, maxs, nof_keys);
			//条件を満たさない領域をbuf2からはずす

			if (debug_flag0 != 0)
			{
				SPRINTF(file, "%s/_5.jpg", work);
				save_image(file, buf2, width, height, 1, 1);
				SPRINTF(file, "%s/_6_%3d.csv", work, th);
				report_toku30(file, toku, label1_n);
			}

			merge(bufm, buf2, label1, toku, width, height);
			//bufmの記録を超えるlabelがあれば、記録とlabel(buf2)を、bufmから更新する

			if (debug_flag0 != 0)
			{
				SPRINTF(file, "%s/_7_%3d.jpg", work, th);
				save_image(file, buf2, width, height, 1, 1);
				SPRINTF(file, "%s/_8_%3d.jpg", work, th);
				save_image_down2(file, bufm, width, height);
			}

			free(toku);
		}

		//最終結果(bufm)を２値化
		for (i = 0; i < height * pitch1; i++)
			if (bufm[i] != 0) buf2[i] = 255;
			else            buf2[i] = 0;

		unsigned char* buf3, *buf4;
		int data, la, r, g, b;
		int rtbl[256], gtbl[256], btbl[256];

		buf3 = (unsigned char*)calloc(height * pitch1, 1);//加工画像

		buf4 = (unsigned char*)calloc(height * pitch3, 1);//加工画像

		//再解析
		Src_label3L(label1, buf2, width, height, &label1_n);//1

		toku = (int*)calloc(4, (label1_n + 1) * DATA_N);

		calc_on_label_img1ch(label1, buf, src_evkey, src_evtype, width, height, toku, label1_n);

		//擬似カラー表示用テーブル
		rtbl[0] = gtbl[0] = btbl[0] = 0;
		for (j = 1; j <= 255; j++)
		{
			i = j * 1023 / 255;
			if (i < 256) { r = 255;      g = i;          b = 0; }
			else if (i < 512) { r = 511 - i;  g = 255;        b = 0; }
			else if (i < 768) { r = 0;        g = 255;        b = (i - 512); }
			else { r = 0;        g = (1023 - i); b = 255; }
			rtbl[j] = r;
			gtbl[j] = g;
			btbl[j] = b;
		}

		//各特徴量解析の図
		for (k = 0; k < nof_keys; k++)
		{
			for (j = 0; j < height; j++)
				for (i = 0; i < width; i++)
				{
					la = label1[j * width + i];//ラベル取得
					if (la == 0)
					{
						buf3[j * pitch1 + i] = 0;//data->規格化->rgb),buf4に記録
						buf4[j * pitch3 + i * 3 + 0] = 0;
						buf4[j * pitch3 + i * 3 + 1] = 0;
						buf4[j * pitch3 + i * 3 + 2] = 0;
						continue;
					}

					data = toku[la * DATA_N + keys[k]];

					if (data < mins[k]) data = 0;
					else if (data > maxs[k]) data = 0;
					else
					{
						data = (data - mins[k]) * 255 / (maxs[k] - mins[k] + 1);
						if (data <= 0) data = 1;
						if (data > 255) data = 255;
					}

					buf3[j * pitch1 + i] = data;//data->規格化->rgb),buf4に記録

					buf4[j * pitch3 + i * 3 + 0] = rtbl[data];
					buf4[j * pitch3 + i * 3 + 1] = gtbl[data];
					buf4[j * pitch3 + i * 3 + 2] = btbl[data];
				}

			//特徴量の記録
			add_num_to_file(file, outfile, 210 + k);
			save_image(file, buf3, width, height, 1, 1);
			add_num_to_file(file, outfile, 250 + k);
			save_image(file, buf4, width, height, 3, 1);
		}

		//特徴量の記録
		add_num_to_file(file, outfile, 300);

		change_extension(file1, file, ".csv");
		printf("file=(%s)\n", file);
		printf("file1=(%s)\n", file1);
		printf("label1_n =%d\n", label1_n);
		printf("%d %d %d %d\n", width, height, channel, depth);

		report_toku30(file1, toku, label1_n);

		//ラベリング結果の表示
		add_char_to_file(file1,outfile,"a");
		add_char_to_file(file2,outfile,"b");
		save_label2(file1,file2,toku,label1_n,label1,width,height,channel);

		free(buf3);
		free(buf4);

		free(toku);
		free(buf);
		free(buf2);
		free(bufm); 

		free(label1);
		free(Buf0);
				
		return(0);
	}
	int save_image_down2(char *file, unsigned short *bufm, int width, int height)
	{
		int i,j,pitch,a;
		unsigned char *buf2;
		long long ss;

		pitch = CALC_PITCH(1, width);

		buf2 = (unsigned char*)calloc(pitch * height, 1);

		ss = 1;
		for (i = 0; i < pitch * height; i++)
			ss += bufm[i];

		ss /= pitch * height;
		ss += 2;

		for (i = 0; i < pitch * height; i++)
		{
			a = bufm[i] * 64 / (ss / 2);
			if (a > 255) a = 255;
			buf2[i] = a;
		}

		save_image(file, buf2, width, height, 1, 1);

		free(buf2);

		return(0);
	}

int save_label2(char *file1,char *file3,int *toku, int label1_n, unsigned long *label1,int width, int height, int channel)
{
		static int lut0[27]={ 0,10, 2, 21, 4,23,  6,16, 8,
			                  9,19,11,  3,13, 5, 15,25,17,
						     18, 1,20, 12,22,14, 24, 7,26};
		int pitch1,pitch3,pitchin,i,j,k,data,color,r,g,b;
		unsigned char *buf1,*buf2,*buf3,*buf4,*buf5;
		char file[512];

		int write_toku_to_bufB(unsigned char* buf3, int* toku2, int key, int label2_n, int width, int height, int fsize, int r, int g, int b);

		pitch1=CALC_PITCH(width,1);
		pitchin=CALC_PITCH(width, channel);
		pitch3=CALC_PITCH(width,3);

//		for (data=0;data<27;data++)//カラーテスト
//		{
//		color=lut0[data%27];//0,1,2->1,2,3
//			r=((color/3)/3)*85+85;
//			g=((color/3)%3)*85+85;
//			b= (color%3)   *85+85;
//					printf ("color=%d rgb= %d %d %d \n",color,r,g,b);
//		}

	buf1=(unsigned char *)calloc(pitch1*height,1);
	buf2=(unsigned char *)calloc(pitch1*height,1);
	buf3=(unsigned char *)calloc(pitch3*height,1);
	buf4=(unsigned char *)calloc(pitch3*height,1);
	buf5=(unsigned char *)calloc(pitch3*height,1);//buf5にBuf0を移す。

//buf5(結果を書き出す背景画像)の準備

	if (channel==3)//このチャネルはBuf0のチャネルです。
	memcpy(buf5, Buf0, pitch3 * height);

	if (channel==4)
	for (j=0;j<height;j++)
	for (i=0;i<width;i++)
	{
		buf5[j * pitch3 + i * 3 + 0] = Buf0[j * pitchin + i * channel + 0];
		buf5[j * pitch3 + i * 3 + 1] = Buf0[j * pitchin + i * channel + 1];
		buf5[j * pitch3 + i * 3 + 2] = Buf0[j * pitchin + i * channel + 2];
	}

	if (channel==1)
	for (j=0;j<height;j++)
	for (i=0;i<width;i++)
	{
		buf5[j * pitch3 + i * 3 + 0] = Buf0[j * pitchin + i + 0];
		buf5[j * pitch3 + i * 3 + 1] = Buf0[j * pitchin + i + 0];
		buf5[j * pitch3 + i * 3 + 2] = Buf0[j * pitchin + i + 0];
	}

		for(j=1;j<height-1;j++)
		for(i=1;i<width-1;i++)
		{
			data=label1[j*width+i];
			if (data == 0)
			{
				continue;
			}

			buf1[j*pitch1+i]=data%128+128;//ラベル番号をグレースケールで表示するbuf
			buf2[j*pitch1+i]=255;//抽出結果だけを白で表示するbuf2

			color=lut0[data%27];//0,1,2->1,2,3
			r=((color/3)/3)*64+64;
			g=((color/3)%3)*64+64;
			b= (color%3)   *64+64;
//printf ("color=%d rgb= %d %d %d \n",color,r,g,b);
			buf3[j*pitch3+i*3+0]=r;//結果を違う色で表示するbuf3
			buf3[j*pitch3+i*3+1]=g;
			buf3[j*pitch3+i*3+2]=b;

			buf4[j*pitch3+i*3+0]=buf5[j*pitch3+i*3+0];//元画像
			buf4[j*pitch3+i*3+1]=buf5[j*pitch3+i*3+1];//抽出領域だけを元画像で表示するbuf4
			buf4[j*pitch3+i*3+2]=buf5[j*pitch3+i*3+2];

			buf5[j*pitch3+i*3+0] = 255;//結果だけを白で表示するbuf5
			buf5[j*pitch3+i*3+1] = 255;
			buf5[j*pitch3+i*3+2] = 255;
		}

		add_num_to_file(file,file1,1);
		save_image(file,buf1,width,height,1,1);

		add_num_to_file(file,file1,2);
		save_image(file,buf2,width,height,1,1);

		add_num_to_file(file,file1,3);
		save_image(file,buf3,width,height,3,1);

		//buf3に番号を記入する。
		write_toku_to_bufB(buf3, toku, 74, label1_n, width, height, 1, 255, 255, 255);

		add_num_to_file(file, file1, 13);
		save_image(file, buf3, width, height, 3, 1);

		add_num_to_file(file,file1,4);
		save_image(file,buf4,width,height,3,1);
		add_num_to_file(file,file1,5);
		save_image(file,buf5,width,height,3,1);

		free(buf1);
		free(buf2);
		free(buf3);
		free(buf4);
		free(buf5);

		return(0);
}

	int load_image(const char* file, int* width, int* height, int* channel, int* depth)
	{
		int width0, height0, channel0, depth0, pitch0;
		int i, j, k, flag;
		Mat image;

		int save_bmp(const char* file, unsigned char* buf, int width0, int height0, int channel0, int depth0);
		int make_hist16(unsigned char* buf, int width, int height, int channel, int* high, int* low);

		image = imread(file, IMREAD_UNCHANGED);//IMREAD_GYARSCALE

		flag = 0;
		check_file_exist(file, &flag);//ファイルの有無確認
		{
			if (flag == 0) return(1);//ファイルがないエラー
		}

		width0 = image.size().width;
		height0 = image.size().height;
		depth0 = image.depth();//CV_8U,CV_16Uか
		channel0 = image.channels();

		if (depth0 == CV_8U) { depth0 = 1; *depth = 1; }//全変数の定義
		else if (depth0 == CV_16U) { depth0 = 2; *depth = 2; }
		*width = width0;
		*height = height0;
		*channel = channel0;
		pitch0 = CALC_PITCH(width0, channel0 * depth0);

		if (*depth == 1)
			Buf0 = (unsigned char*)calloc(sizeof(unsigned char), pitch0 * height0);
		if (*depth == 2)
			Buf0 = (unsigned char*)calloc(sizeof(unsigned char), pitch0 * height0);

		//ファイルのサイズ確認
		printf("%d %d %d %d %d\n", width0, height0, channel0, depth0, pitch0);

		//Buf0にデータを移し替える

		if (depth0 == 1)
			for (j = 0; j < height0; j++)//image.dataはrawのべた書き
				for (i = 0; i < width0; i++)
					for (k = 0; k < channel0; k++)
						Buf0[j * pitch0 + i * channel0 + k] = image.data[(j * width0 + i) * channel0 + k];

		if (depth0 == 2)
			for (j = 0; j < height0; j++)//image.dataはrawのべた書き
				for (i = 0; i < width0; i++)
					for (k = 0; k < channel0; k++)
						* (unsigned short*) & (Buf0[j * pitch0 + i * channel0 * 2 + k * 2]) = *(unsigned short*) & (image.data[((j * width0 + i) * channel0 + k) * 2]);

		printf("Buf0[0]=%d\n", Buf0[0]);

		//	memcpy(Buf0,image.data, pitch0*height0);
		//	save_bmp("b.bmp",Buf0,width0,height0,channel0,depth0);

		image.release();

		return(0);
	}
	int save_image(char* file0, unsigned char* buf, int width, int height, int channel, int depth)
	{
		int pitch, i, j, k, bit_type;
		unsigned char* buf2;
		char file[512], extension[512], file_plain_name[512];

		if (depth == 2)//tif形式でない場合
		{
			get_file_extension(file0, extension, file_plain_name);
			if ((strcmp(extension, ".tiff") != 0) && (strcmp(extension, ".tif") != 0) &&
				(strcmp(extension, ".TIF") != 0) && (strcmp(extension, ".TIFF") != 0))
			{
				SPRINTF(file, "%s.tif", file0);
			}
			else { STRCPY(file, file0); }
		}
		else { STRCPY(file, file0); }

		//printf ("file=%s\n",file);
		//printf ("file0=%s\n",file0);PL();

		buf2 = (unsigned char*)calloc(1, width * height * channel * depth);

		if ((depth == 1) && (channel == 1)) bit_type = CV_8UC1;
		else if ((depth == 1) && (channel == 3)) bit_type = CV_8UC3;
		else if ((depth == 2) && (channel == 1)) bit_type = CV_16UC1;
		else if ((depth == 2) && (channel == 3)) bit_type = CV_16UC3;
		else if ((depth == 4) && (channel == 1)) bit_type = CV_8UC1;//ラベル表示

		Mat image(Size(width, height), bit_type, buf2);

		pitch = CALC_PITCH(width, channel * depth);

		//	printf ("save:(%s)(w=%d H=%d depth=%d channel=%d)\n",file,image.size().width,image.size().height,
		//		image.depth(),image.channels());

			//入れ替え
		if (depth == 1)
			for (j = 0; j < height; j++)
				for (i = 0; i < width; i++)
					for (k = 0; k < channel; k++)
					{
						//		printf ("%d %d\n",(j*width+i)*channel+k,image.data[(j*width+i)*channel+k]);
						image.data[(j * width + i) * channel + k] = buf[j * pitch + i * channel + k];
					}

		//入れ替え
		if (depth == 2)
			for (j = 0; j < height; j++)
				for (i = 0; i < width; i++)
					for (k = 0; k < channel; k++)
					{
						//printf ("%d %d\n",(j*width+i)*channel+k,image.data[(j*width+i)*channel+k]);
						*(unsigned short*)& (image.data[((j * width + i) * channel + k) * 2])
							= *(unsigned short*) & (buf[j * pitch + (i * channel + k) * 2]);
					}

		imwrite(file, image);

		free(buf2);
		image.release();

		return(0);
	}
	int add_num_to_file(char* outfile1, char* outfile, int num)
	{
		char path[512], pname[512], name[512], extension[512];

		get_file_name_and_dir(outfile, name, path);
		get_file_extension(name, extension, pname);

		SpRINTF7(outfile1, 512, "%s/%s_%d%s", path, pname, num, extension);
		//printf ("(%s)\n",outfile1);

		return(0);
	}
	int read_src_param(char* parafile, int* posi_nega, int* rate, int* step, int* evkey, int* evflag, int* keys, int* mins, int* maxs, int* para_n) //パラメータ読み込み
	{
		int i, n;
		char linebuf[1024];
		FILE* fp;
		double para[50];

		//FOPEN(fp, parafile, "r");//windows 5
		fp = FOPEN(parafile, "r");//linux 5

		for (i = 0; i < 50; i++)
		{
			if (fgets(linebuf, sizeof(linebuf), fp) == NULL) break;
			SSCANF(linebuf, "%lf", &para[i]);
			if ((i >= 9) && ((int)para[i - 1] == 0) && ((int)para[i] == 0)) break;
		}
		printf("n=%d\n", i);
		n = i;//個数

		fclose(fp);

		//			for (i=0;i<n;i++)
		//			printf ("%lf ",para[i]) ;
		//			printf ("\n");

		//読み替え
		*posi_nega = (int)para[0];
		*rate = (int)para[1];
		*step = (int)para[2];
		*evkey = (int)para[3];
		*evflag = (int)para[4];

		*para_n = (n - 5) / 3;
		for (i = 0; i < *para_n; i++)
		{
			keys[i] = (int)para[5 + i * 3];
			mins[i] = (int)para[5 + i * 3 + 1];
			maxs[i] = (int)para[5 + i * 3 + 2];
		}

		printf("posi_nega=%d,rate=%d,step=%d,evkey=%d,evflag=%d\n",
			*posi_nega, *rate, *step, *evkey, *evflag);

		for (i = 0; i < *para_n; i++)
			printf("%d: %d %d %d \n", i, keys[i], mins[i], maxs[i]);

		return(0);
	}
	int check_file_exist(const char* file, int* flag)//ファイルチェックしないとexitする
	{
		FILE* fp;

		//FOPEN(fp, file, "rb");//windows 6
		fp = FOPEN(file, "rb");//linux 6

		if (fp == nullptr)//error
		{
			*flag = 0;
			return(1);
		}
		else //safe
		{
			*flag = 1;
			fclose(fp);
			return(0);
		}
	}
	int check_param(unsigned long* label0, unsigned char* buf2, int width, int height, int* toku, int label1_n, int* keys, int* mins, int* maxs, int para_n)
	{
		int i, j, k, v, pitch, lab;

		for (i = 1; i <= label1_n; i++)//駄目なものをチェックする
		{
			for (k = 0; k < para_n; k++)
			{
				v = toku[i * DATA_N + keys[k]];
				if ((v >= mins[k]) && (v <= maxs[k])) {}
				else {
					toku[i * DATA_N + DATA_N - 1] = -1;
					toku[i * DATA_N + DATA_N - 5] = keys[k];//DATA_N-3,DATA_N-2番には適さない理由を書く
					toku[i * DATA_N + DATA_N - 4] = mins[k];//DATA_N-3,DATA_N-2番には適さない理由を書く
					toku[i * DATA_N + DATA_N - 3] = maxs[k];
					break;
				}//1個でも条件を満たさなければ抜ける。
			}
		}

		pitch = CALC_PITCH(1, width);

		for (j = 0; j < height; j++)
			for (i = 0; i < width; i++)
			{
				lab = label0[j * width + i];
				if (lab == 0) continue;
				if (toku[lab * DATA_N + DATA_N - 1] == -1)  buf2[j * pitch + i] = 0;
			}

		return(0);
	}

int report_toku30(char *file,int *toku,int label1_n)
{
	int i;
	FILE *fp;

	printf("file=(%s)\n",file);
	printf("label1_n=%d\n", label1_n);

	//FOPEN(fp,file,"w");//windows 6
	fp = fopen(file, "w");//linux 6

	fprintf (fp,"no,0:pixels,1:point-x,2:point-y,3:0x,4:x1,5:y0,6:y1,7:width,8:height,");
	fprintf(fp, "9:inv-oblate,10:S-length,11:L-length1,12:oblate,13:round,14:ellipsity,15:filling-ratio,16:outer-pixels,");
	fprintf(fp, "17:thickness,18:slenderness,19:wrap-count,20:outer-area,21:deg0,22:deg1,");
	fprintf (fp,"23:average,24:min-v,25:max-v,26:contrast1,27:ex-average,28:contrast2,29:contrast3,\n");
	for(i=1;i<=label1_n;i++)
	{
fprintf (fp,"%d, %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, ",
i,
toku[i * DATA_N+ 0],toku[i * DATA_N + 1], toku[i * DATA_N + 2], toku[i * DATA_N + 3], toku[i * DATA_N + 4],
toku[i * DATA_N+ 5],toku[i * DATA_N + 6], toku[i * DATA_N + 7], toku[i * DATA_N + 8], toku[i * DATA_N +39]);

fprintf(fp, " %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, ", 
toku[i * DATA_N+40],toku[i * DATA_N +41], toku[i * DATA_N +42], toku[i * DATA_N +43], toku[i * DATA_N +44],
toku[i * DATA_N+45],toku[i * DATA_N +46], toku[i * DATA_N +47], toku[i * DATA_N +48], toku[i * DATA_N +49]);

fprintf(fp, " %d,%d,%d,%d,%d, %d,%d,%d,%d,%d, \n", 
toku[i * DATA_N+50],toku[i * DATA_N +51], toku[i * DATA_N +52], toku[i * DATA_N +60], toku[i * DATA_N +61],
toku[i * DATA_N+62],toku[i * DATA_N +63], toku[i * DATA_N +64], toku[i * DATA_N +65], toku[i * DATA_N +66]);
	}

	if (fp!=NULL)
	fclose(fp);

	return(0);
}
	int merge(unsigned short* buf3, unsigned char* buf2, unsigned long* label0, int* toku, int width, int height)
	{
		int pitch, i, j, lab, ev;

		pitch = CALC_PITCH(1, width);

		//labelが0でないものの、labとtoku->buf3のevと比較し、強さ比較

		for (j = 0; j < height; j++)
			for (i = 0; i < width; i++)
			{
				lab = label0[j * width + i];
				if (lab == 0) continue;
				if (toku[lab * DATA_N + DATA_N - 1] == -1) {}//無効
				else//有効
				{
					ev = toku[lab * DATA_N + DATA_N - 2];//評価値
					if (ev < buf3[j * pitch + i]) toku[lab * DATA_N + DATA_N - 1] = -1;//フラグを落とす(1つでも負ければ)
				}
			}

		for (j = 0; j < height; j++)
			for (i = 0; i < width; i++)
			{
				lab = label0[j * width + i];
				if (lab == 0) continue;
				if (toku[lab * DATA_N + DATA_N - 1] == -1) {}//無効
				else
				{
					ev = toku[lab * DATA_N + DATA_N - 2];//評価値
					buf3[j * pitch + i] = ev;//フラグを書き込む(マージしたという意味)
				}
			}

		return(0);
	}
	int add_char_to_file(char* outfile1, char* outfile, const char* cha)
	{
		char path[512], pname[512], name[512], extension[512];

		get_file_name_and_dir(outfile, name, path);
		get_file_extension(name, extension, pname);

		SpRINTF7(outfile1, 512, "%s/%s%s%s", path, pname, cha, extension);
		//printf ("(%s)\n",outfile1);

		return(0);
	}
	int calc_on_label_img1ch(unsigned long* label1, unsigned char* buf, int key, int value1, int width, int height, int* toku, int label1_n)
	{
		//寸法系
		//			toku[0] = 0;//カウンタ
		//			toku[1] +=i;//x重心
		//			toku[2] +=j;//y重心
		// 	   		toku[3] = xmin;
		//			toku[4] = xmax;
		//			toku[5] = ymin;
		//			toku[6] = ymax;
		//			toku[7] = width0;
		//			toku[8] = height0;
		// 	   		toku[9] = len[0]; //0度方向長さ
		//			toku[9+i]=len[i]; //i*11度方向長さ
		//			toku[24]= len[15];//15*11方向長さ
		// 	        toku[25];//カウントの10000分率
		// 	        toku[25+i];//特徴量iの10000分率
		//	        toku[33+i];//height0の10000分率
		//34-38:欠番
		// 	   		toku[39] = maxL*10000/minL;//逆扁平率
		//			toku[40] = minL;//短辺
		//			toku[41] = maxL;//長辺
		//			toku[42] = minL * 10000 / maxL;//扁平率
		// 
		// 	   		toku[43] =count*1000.0/(maxL*maxL*3.14159/2.0/2.0)+0.5);//円形度
		//			toku[44] =count*1000.0/(minL*maxL*3.14159/2.0/2.0)+0.5);//楕円度
		// 			toku[45] =round;//四角度
		//			toku[46] =round;//外周(63と同じ）
		//			toku[47] =count*2/(round);//平均太さ
		//			toku[48] =count/maxL;//細長さ＝長さ/太さ
		//			toku[49] =round*1000/(minL+maxL)/2;//折り返し数
		//			toku[50] =mindeg;//向き(度)
		//			toku[51] =count/round;//外周面積比
		//領域(w,h):重複
		//			toku[53] x0
		//			toku[54] x1
		//			toku[55] y0
		//			toku[56] y1
		//			toku[57] = toku[i * DATA_N + 54] - toku[i * DATA_N + 53] + 1;//width0
		//			toku[58] = toku[i * DATA_N + 56] - toku[i * DATA_N + 55] + 1;//height0
		//値系
		//			toku[60] bufの値;//平均値
		//			toku[61] min　bufの値
		//			toku[62] max　bufの値
		// 			toku[63] =maxbuf-minbuf //内部コントラスト（内部の最大ー内部の最小）
		//コントラスト系
		//			toku[64]/=toku[i * DATA_N +63];//直外画素の平均値
		// 			toku[65]=toku[i * DATA_N +60]-toku[i * DATA_N +64];//内外コントラスト(中平均と直外側の差）
		//			toku[66]=toku[i * DATA_N +61]-toku[i * DATA_N +64];//内外コントラスト（中最小値と直外値)
		//			toku[74] = i;//ラベル番号
		//評価関数用
		//			toku[80] = toku[ 0]*toku[43]
		// 			toku[81] = toku[43]*toku[65]
		// 			toku[82] = toku[65]*toku[ 0]
		//制御変数
		//		toku[DATA_N-5]=key;不合格理由 //95
		//		toku[DATA_N-4]=min0;不合格理由 //96
		//		toku[DATA_N-3]=max0;不合格理由 //97
		//		toku[DATA_N-2]=65535-abs(toku[i * DATA_N +key]-value);評価関数 //98
		//		toku[DATA_N-1]=0,-1;領域結合フラグ //99

		//画素値を含む特徴量
		int i, j, la, pitch, count, width0, height0;
		int minL, maxL, round, count2, v;
		int deg, len[16], xmin, xmax, ymin, ymax, mindeg, maxdeg;
		double sin0[20], cos0[20];
		double en, juu, juu0;
		int daen, daen0, minSS;

		int set_sin_cos(double* sin, double* cos);

		pitch = CALC_PITCH(1, width);

		set_sin_cos(sin0, cos0);//三角関数表

		//初期化
		for (i = 1; i <= label1_n; i++)
		{
			for (j = 0; j < DATA_N; j++) toku[i * DATA_N + j] = 0;
			toku[i * DATA_N + 53] = width - 1;//最小値
			toku[i * DATA_N + 55] = height - 1;//最小値

			for (deg = 0; deg < 16; deg++)//3->34
			{
				toku[i * DATA_N + 3 + deg * 2] = 999999999;//x_min
				toku[i * DATA_N + 4 + deg * 2] = -999999999;//x_max
			}
			toku[i * DATA_N + 61] = 999999999;//y
		}

		for (j = 0; j < height; j++)
			for (i = 0; i < width; i++)
			{
				la = label1[j * width + i];
				if (la == 0) continue;

				toku[la * DATA_N + 0]++;//カウンタ加算(最初数だけ数える)

				if (i < toku[la * DATA_N + 53]) toku[la * DATA_N + 53] = i;
				if (i > toku[la * DATA_N + 54]) toku[la * DATA_N + 54] = i;
				if (j < toku[la * DATA_N + 55]) toku[la * DATA_N + 55] = j;
				if (j > toku[la * DATA_N + 56]) toku[la * DATA_N + 56] = j;

				if (toku[la * DATA_N + 1] < 2000000000) toku[la * DATA_N + 1] += i;
				if (toku[la * DATA_N + 2] < 2000000000) toku[la * DATA_N + 2] += j;

				for (deg = 0; deg < 16; deg++)
					//len[deg] = (int)(sin0[deg] * (double)i + cos0[deg] * (double)j);
					len[deg] = (int)(cos0[deg] * (double)i - sin0[deg] * (double)(-j));
				//printf("1 %d %d %d %d %d %d\n", i, j, len[0], len[1], len[2], len[3]);

				for (deg = 0; deg < 16; deg++)//投影長(min)
					if (len[deg] < toku[la * DATA_N + 3 + deg * 2]) { toku[la * DATA_N + 3 + deg * 2] = len[deg]; }

				for (deg = 0; deg < 16; deg++)//投影長(max)
					if (len[deg] > toku[la * DATA_N + 4 + deg * 2]) { toku[la * DATA_N + 4 + deg * 2] = len[deg]; }

				v = buf[j * pitch + i];//グレースケールの数値
				if (toku[la * DATA_N + 60] < 2000000000) toku[la * DATA_N + 60] += v;//数値

				if (v < toku[la * DATA_N + 61]) toku[la * DATA_N + 61] = v;//min
				if (v > toku[la * DATA_N + 62]) toku[la * DATA_N + 62] = v;//max

				//近隣に別のラベルがあれば(8方向)
				if ((i != 0) && (j != 0) && (i != width - 1) && (j != height - 1))
				{
					//近隣に別のラベルがあれば(4方向),外側の点の値加算,63は個数,64は数値,
					if (label1[(j - 1) * width + (i)] == 0) { toku[la * DATA_N + 63]++; toku[la * DATA_N + 64] += buf[(j - 1) * pitch + (i)]; }
					if (label1[(j)* width + (i - 1)] == 0) { toku[la * DATA_N + 63]++; toku[la * DATA_N + 64] += buf[(j)* pitch + (i - 1)]; }
					if (label1[(j)* width + (i + 1)] == 0) { toku[la * DATA_N + 63]++; toku[la * DATA_N + 64] += buf[(j)* pitch + (i + 1)]; }
					if (label1[(j + 1) * width + (i)] == 0) { toku[la * DATA_N + 63]++; toku[la * DATA_N + 64] += buf[(j + 1) * pitch + (i)]; }
				}
			}

		for (i = 1; i <= label1_n; i++)
		{
			count = toku[i * DATA_N + 0];
			round = toku[i * DATA_N + 63];//周囲長
			if (count == 0) count++;
			if (round == 0) round++;

			toku[i * DATA_N + 1] = toku[i * DATA_N + 1] / count;//重心
			toku[i * DATA_N + 2] = toku[i * DATA_N + 2] / count;//重心

			for (deg = 0; deg < 16; deg++) //16方位長を退避
				len[deg] = (toku[i * DATA_N + 4 + deg * 2] - toku[i * DATA_N + 3 + deg * 2]) / 1000 + 1;

			toku[i * DATA_N + 57] = toku[i * DATA_N + 54] - toku[i * DATA_N + 53] + 1;//width0
			toku[i * DATA_N + 58] = toku[i * DATA_N + 56] - toku[i * DATA_N + 55] + 1;//height0

			xmin = toku[i * DATA_N + 53];
			xmax = toku[i * DATA_N + 54];
			ymin = toku[i * DATA_N + 55];
			ymax = toku[i * DATA_N + 56];
			width0 = toku[i * DATA_N + 57];
			height0 = toku[i * DATA_N + 58];

			toku[i * DATA_N + 3] = xmin;
			toku[i * DATA_N + 4] = xmax;
			toku[i * DATA_N + 5] = ymin;
			toku[i * DATA_N + 6] = ymax;
			toku[i * DATA_N + 7] = width0;
			toku[i * DATA_N + 8] = height0;

			for (deg = 0; deg < 16; deg++) //16方位長
				toku[i * DATA_N + 9 + deg] = len[deg];

			toku[i * DATA_N + 25] = (int)((double)count / (double)(width * height) * 10000.0 + 0.5);
			toku[i * DATA_N + 26] = (int)((double)toku[i * DATA_N + 1] / (double)(width) * 10000.0 + 0.5);
			toku[i * DATA_N + 27] = (int)((double)toku[i * DATA_N + 2] / (double)(height) * 10000.0 + 0.5);
			toku[i * DATA_N + 28] = (int)((double)toku[i * DATA_N + 3] / (double)(width) * 10000.0 + 0.5);
			toku[i * DATA_N + 29] = (int)((double)toku[i * DATA_N + 4] / (double)(width) * 10000.0 + 0.5);
			toku[i * DATA_N + 30] = (int)((double)toku[i * DATA_N + 5] / (double)(height) * 10000.0 + 0.5);
			toku[i * DATA_N + 31] = (int)((double)toku[i * DATA_N + 6] / (double)(height) * 10000.0 + 0.5);
			toku[i * DATA_N + 32] = (int)((double)toku[i * DATA_N + 7] / (double)(width) * 10000.0 + 0.5);
			toku[i * DATA_N + 33] = (int)((double)toku[i * DATA_N + 8] / (double)(height) * 10000.0 + 0.5);
			toku[i * DATA_N + 34] = 0;

			minL = toku[i * DATA_N + 9];//短辺
			mindeg = 0;
			maxdeg = 90;
			for (deg = 0; deg < 16; deg++)
				if (toku[i * DATA_N + 9 + deg] < minL)
				{
					minL = toku[i * DATA_N + 9 + deg];
					mindeg = (int)((double)deg * 11.25 + 0.5);
				}
			if ((mindeg == 0) && (width0 == height0)) mindeg = 180;

			maxL = toku[i * DATA_N + 9];//短辺
			for (deg = 0; deg < 16; deg++)
				if (toku[i * DATA_N + 9 + deg] > maxL)
				{
					maxL = toku[i * DATA_N + 9 + deg];
					maxdeg = (int)((double)deg * 11.25 + 0.5);
				}

			if (minL == 0) minL++;//ゼロ割対策
			if (maxL == 0) maxL++;

			toku[i * DATA_N + 39] = maxL * 10000 / minL;//逆扁平率
			toku[i * DATA_N + 40] = minL;//短辺
			toku[i * DATA_N + 41] = maxL;//長辺
			toku[i * DATA_N + 42] = minL * 10000 / maxL;//扁平率

			//円形度(maxL基準充填度)
			en = (double)maxL * (double)maxL * (3.14159 / 2.0 / 2.0);
			toku[i * DATA_N + 43] = (int)(((double)((double)count / en) * 10000.0) + 0.5);//円形度
			if (toku[i * DATA_N + 43] > 10000) toku[i * DATA_N + 43] = 10000;
			if (count == 1) toku[i * DATA_N + 43] = 1;//大きさ1で円形度がmaxならないように
			//最長辺を直径とする円と、面積の比(円なら10000それ以外は、小さい数字)

			daen = 0;//楕円度
			for (deg = 0; deg < 8; deg++)
			{
				daen0 = toku[i * DATA_N + 9 + deg] * toku[i * DATA_N + 17 + deg];
				daen0 = (int)(((double)((double)count / (double)daen0 / (3.14159 / 2.0 / 2.0)) * 10000.0) + 0.5);//楕円度
				if (daen0 > daen) daen = daen0;
			}
			toku[i * DATA_N + 44] = daen;
			//楕円に対する、面積の比(円なら10000それ以外は、小さい数字)

			toku[i * DATA_N + 45] = (int)((double)count * 10000.0 / (double)(width0 * height0));//四角度

			toku[i * DATA_N + 46] = round;//外周を保存
			toku[i * DATA_N + 47] = (int)((double)count * 2.0 / (double)round + 0.5);//太さ
			if (toku[i * DATA_N + 47] == 0) toku[i * DATA_N + 47]++;
			toku[i * DATA_N + 48] = (int)((double)round / ((double)(toku[i * DATA_N + 47]) * 2.0) + 0.5);//細長さ
			if (toku[i * DATA_N + 47] == 0) toku[i * DATA_N + 47] = 1;
			if (toku[i * DATA_N + 48] == 0) toku[i * DATA_N + 48] = 1;

			toku[i * DATA_N + 49] = (minL + maxL) * 20000 / round;//折り返す回数
			if (count < 1000000) //オーバーフロー対策
			{
				toku[i * DATA_N + 50] = (count * 100) / round;
			}//外周面積比
			else
			{
				toku[i * DATA_N + 50] = count / (round / 100 + 1);
			}//外周面積比
			toku[i * DATA_N + 51] = mindeg;//外周面積比
			toku[i * DATA_N + 52] = maxdeg;//向き

			toku[i * DATA_N + 60] /= count;//平均値
			//61 min
			//62 max
			toku[i * DATA_N + 63] = toku[i * DATA_N + 62] - toku[i * DATA_N + 61];//con_inner内部差
			toku[i * DATA_N + 64] /= round;//外平均
			toku[i * DATA_N + 65] = toku[i * DATA_N + 60] - toku[i * DATA_N + 64];//内外コントラスト(中平均と直外側の差）
			toku[i * DATA_N + 66] = toku[i * DATA_N + 60] - toku[i * DATA_N + 61];//内外コントラスト(中最小値と直外値の差)

			toku[i * DATA_N + 74] = i;

			toku[i * DATA_N + 80] = (int)sqrt(sqrt(toku[i * DATA_N + 0]) * toku[i * DATA_N + 43]);
			toku[i * DATA_N + 81] = (int)sqrt(toku[i * DATA_N + 43] * toku[i * DATA_N + 65]);
			toku[i * DATA_N + 82] = (int)sqrt(toku[i * DATA_N + 65] * sqrt(toku[i * DATA_N + 0]));
			toku[i * DATA_N + 83] = (int)sqrt(toku[i * DATA_N + 63] * sqrt(toku[i * DATA_N + 0]));

			if (key == 0)
			{
				if (value1 == -1)//大きいほど良い
					toku[i * DATA_N + DATA_N - 2] = (int)(sqrt((double)toku[i * DATA_N + key]));
				else if (value1 == 0)//小さいほど良い
					toku[i * DATA_N + DATA_N - 2] = 65535 - ((int)sqrt((double)(abs(toku[i * DATA_N + key]))));
				else
					toku[i * DATA_N + DATA_N - 2] = 65535 - ((int)sqrt((double)(abs(toku[i * DATA_N + key] - sqrt((double)value1)))));
			}
			else
			{
				if (value1 == -1)//大きいほど良い
					toku[i * DATA_N + DATA_N - 2] = abs(toku[i * DATA_N + key]);
				else if (value1 == 0)//小さいほど良い
					toku[i * DATA_N + DATA_N - 2] = 65535 - abs(toku[i * DATA_N + key]);
				else
					toku[i * DATA_N + DATA_N - 2] = 65535 - abs(toku[i * DATA_N + key] - value1);
			}
			/*
			int kk;

				for (kk=0;kk<DATA_N;kk++)
				{
				printf("%d ",toku[i * DATA_N +kk]);
				if (kk%10==0) printf ("\n") ;
				}
				 printf ("\n") ;
			*/
		}
		return(0);
	}
	int change_extension(char* outfile1, char* outfile, const char* ext)
	{
		char path[512], pname[512], name[512], extension[512];

		get_file_name_and_dir(outfile, name, path);
		get_file_extension(name, extension, pname);

		SpRINTF6(outfile1, 512, "%s/%s%s", path, pname, ext);
		//printf ("(%s)\n",outfile1);

		return(0);
	}
	int get_file_name_and_dir(char* full_path, char* file_name, char* file_dir)
	{
		int len, i, p;

		//ディレクトリの区切りを探す
		len = (int)strlen(full_path);
		for (i = len - 1; i >= 0; i--)
			if (full_path[i] == '/') break;

		p = i;//区切り位置はp

		//file_dirに関してはp番目までをコピーして、最後を終端。
		//file_nameに関してはp番目以降をコピーして、最後を終端。

		for (i = 0; i < p; i++)
			file_dir[i] = full_path[i];
		file_dir[i] = 0;//ここはfor文の外、次の要素をゼロにする。（終端)
		if (strlen(file_dir) < 1)//ファイル名がカレントパスの場合
		{
			//_getcwd(file_dir, 512);//windows 8
			getcwd(file_dir, sizeof(file_dir)); //linux 8
		}

		for (i = p + 1; i < len; i++)
			file_name[i - p - 1] = full_path[i];
		file_name[i - p - 1] = 0;//ここはfor文の外、次の要素をゼロにする。(終端)

		return(0);
	}
	int set_sin_cos(double* sin0, double* cos0)
	{
		int i;

		sin0[0] = 0.000000; cos0[0] = 1.000000;
		sin0[1] = 0.195090; cos0[1] = 0.980785;
		sin0[2] = 0.382683; cos0[2] = 0.923879;
		sin0[3] = 0.555570; cos0[3] = 0.831469;

		sin0[4] = 0.707106; cos0[4] = 0.707106;
		sin0[5] = 0.831469; cos0[5] = 0.555570;
		sin0[6] = 0.923879; cos0[6] = 0.382683;
		sin0[7] = 0.980785; cos0[7] = 0.195090;

		sin0[8] = 1.000000; cos0[8] = 0.000000;
		sin0[9] = 0.980785; cos0[9] = -0.195090;
		sin0[10] = 0.923879; cos0[10] = -0.382683;
		sin0[11] = 0.831469; cos0[11] = -0.555570;

		sin0[12] = 0.707106; cos0[12] = -0.707106;
		sin0[13] = 0.555570; cos0[13] = -0.831469;
		sin0[14] = 0.382683; cos0[14] = -0.923879;
		sin0[15] = 0.195090; cos0[15] = -0.980785;

		for (i = 0; i < 16; i++) sin0[i] = 1000.0 * sin0[i];
		for (i = 0; i < 16; i++) cos0[i] = 1000.0 * cos0[i];

		return(0);
	}
	int get_file_extension(char* file_name, char* file_extension, char* file_plain_name)
	{
		int i, len, p;

		len = (int)strlen(file_name);

		//後ろからサーチして'.'を探す。
		for (i = len - 1; i >= 0; i--)
			if (file_name[i] == '.') break;

		p = i;

		//file_plain_nameに関してはp番目までをコピーして、最後を終端。
		//file_extensionに関してはp番目以降をコピーして、最後を終端。

		for (i = 0; i < p; i++)
			file_plain_name[i] = file_name[i];

		//printf("1(%s)\n", file_plain_name);

		file_plain_name[p] = 0;//ここはfor文の外、次の要素をゼロにする。（終端)

	//	printf("\t1(%s)\n", file_plain_name);

		for (i = p; i < len; i++)
			file_extension[i - p] = file_name[i];
		file_extension[i - p] = 0;//ここはfor文の外、次の要素をゼロにする。(終端)
	//	file_extension[i-p+1]=0;//ここはfor文の外、次の要素をゼロにする。(終端)

		return(0);
	}
	int img_choose_white(unsigned char* buf1, int min, int width, int height)
	{
		//孤立した黒点を埋める
		unsigned long* label;
		int label1_n, i, j, pitch;
		int* count;

		pitch = CALC_PITCH(1, width);

		label = (unsigned long*)calloc(sizeof(unsigned long), pitch * height);//ラベリング領域を確保する

		Src_label2L(label, buf1, width, height, &label1_n);//buf0をラベリングする。

		count = (int*)calloc(sizeof(int), label1_n + 10);//カウント領域を確保する。

		for (j = 0; j < height; j++)//画素数をカウントする
			for (i = 0; i < width; i++)
				if (label[j * pitch + i] != 0)
					count[label[j * pitch + i]]++;

		for (j = 0; j < height; j++)
			for (i = 0; i < width; i++)
				if (label[j * pitch + i] != 0)
					if (count[label[j * pitch + i]] > min) buf1[j * pitch + i] = 0;//特定画素より大きい領域を消す

		free(label);
		free(count);

		return (0);
	}
	int Src_label3L(unsigned long* label1, unsigned char* buf74, int width, int height, int* label1_n)
	{
		//ラベリング処理（関数部のサブ関数）
		int label_num;
		int l[4];
		int max, pitch;
		int i, j;
		int current;
		int x, y;
		unsigned long* work, * tl;

		pitch = CALC_PITCH(width, 1);

		//四隅を除去する
		j = 0;
		for (i = 0; i < width; i++)
			label1[j * width + i] = 0;

		j = height - 1;
		for (i = 0; i < width; i++)
			label1[j * width + i] = 0;

		i = 0;
		for (j = 0; j < height; j++)
			label1[j * width + i] = 0;

		i = width - 1;
		for (j = 0; j < height; j++)
			label1[j * width + i] = 0;

		work = (unsigned long*)calloc(height * width + width, sizeof(unsigned long));//作業用バッファ
		tl = (unsigned long*)calloc(height * width + width / 4, sizeof(unsigned long));//作業用バッファ原文は1/4

		for (i = 0; i < height * width; i++) work[i] = 0;
		for (i = 0; i < height * width; i++) tl[i] = 0;

		current = 0;//現在のラベル数

		l[0] = l[1] = l[2] = l[3] = 0;

		for (y = 1; y < height - 1; y++)
			for (x = 1; x < width - 1; x++)
				if (buf74[y * pitch + x] == 255)
				{
					l[0] = work[(y - 1) * width + (x - 1)]; while (tl[l[0]] != 0) { l[0] = tl[l[0]]; }
					l[1] = work[(y - 1) * width + (x)]; while (tl[l[1]] != 0) { l[1] = tl[l[1]]; }
					l[2] = work[(y - 1) * width + (x + 1)]; while (tl[l[2]] != 0) { l[2] = tl[l[2]]; }
					l[3] = work[(y)* width + (x - 1)]; while (tl[l[3]] != 0) { l[3] = tl[l[3]]; }

					max = l[0];
					if (l[1] > max) max = l[1];
					if (l[2] > max) max = l[2];
					if (l[3] > max) max = l[3];

					if (max == 0)
					{
						current++;
						work[y * width + x] = current;
					}
					else
					{
						work[y * width + x] = max;
						if (l[0] != 0 && l[0] != max) { tl[l[0]] = max; }
						if (l[1] != 0 && l[1] != max) { tl[l[1]] = max; }
						if (l[2] != 0 && l[2] != max) { tl[l[2]] = max; }
						if (l[3] != 0 && l[3] != max) { tl[l[3]] = max; }
					}
				}

		label_num = 0;
		for (i = 1; i <= current; i++)
		{
			j = tl[i];
			while (j != 0)
			{
				tl[i] = j;
				j = tl[j];
			}
		}

		for (i = 1; i <= current; i++)
		{
			if (tl[i] == 0)
			{
				label_num++;
				for (j = 1; j <= current; j++)
				{
					if (tl[j] == i) { tl[j] = label_num; }
				}
				tl[i] = label_num;
			}
		}

		// 最終的に、ラベル値のセット
		for (y = 0; y < height; y++)
			for (x = 0; x < width; x++)
			{
				if (work[y * width + x])
				{
					label1[y * width + x] = tl[work[y * width + x]];
				}
				else
				{
					label1[y * width + x] = 0;
				}
			}

		*label1_n = label_num;

		free(work);
		free(tl);

		return (0);
	}
	int Src_label2L(unsigned long* label1, unsigned char* buf74, int width, int height, int* label1_n)
	{
		//ラベリング処理（関数部のサブ関数）
		int label_num;
		int l[4];
		int max;
		int i, j;
		int current;
		int x, y, pitch;
		unsigned long* work, * tl;

		pitch = CALC_PITCH(1, width);

		work = (unsigned long*)calloc(sizeof(unsigned long), height * pitch + pitch);//作業用バッファ
		tl = (unsigned long*)calloc(sizeof(unsigned long), (height * pitch + pitch) / 4);//作業用バッファ原文は1/4

		for (i = 0; i < height * pitch; i++) work[i] = 0;
		for (i = 0; i < height * pitch / 4; i++) tl[i] = 0;

		current = 0;//現在のラベル数

		l[0] = l[1] = l[2] = l[3] = 0;

		for (y = 1; y < height - 1; y++)
			for (x = 1; x < width - 1; x++)
				if (buf74[y * pitch + x] == 255)
				{
					l[0] = work[(y - 1) * pitch + (x - 1)]; while (tl[l[0]] != 0) { l[0] = tl[l[0]]; }
					l[1] = work[(y - 1) * pitch + (x)]; while (tl[l[1]] != 0) { l[1] = tl[l[1]]; }
					l[2] = work[(y - 1) * pitch + (x + 1)]; while (tl[l[2]] != 0) { l[2] = tl[l[2]]; }
					l[3] = work[(y)* pitch + (x - 1)]; while (tl[l[3]] != 0) { l[3] = tl[l[3]]; }

					max = l[0];
					if (l[1] > max) max = l[1];
					if (l[2] > max) max = l[2];
					if (l[3] > max) max = l[3];

					if (max == 0)
					{
						current++;
						work[y * pitch + x] = current;
					}
					else
					{
						work[y * pitch + x] = max;
						if (l[0] != 0 && l[0] != max) { tl[l[0]] = max; }
						if (l[1] != 0 && l[1] != max) { tl[l[1]] = max; }
						if (l[2] != 0 && l[2] != max) { tl[l[2]] = max; }
						if (l[3] != 0 && l[3] != max) { tl[l[3]] = max; }
					}
				}

		label_num = 0;
		for (i = 1; i <= current; i++)
		{
			j = tl[i];
			while (j != 0)
			{
				tl[i] = j;
				j = tl[j];
			}
		}

		for (i = 1; i <= current; i++)
		{
			if (tl[i] == 0)
			{
				label_num++;
				for (j = 1; j <= current; j++)
				{
					if (tl[j] == (unsigned int)i) { tl[j] = label_num; }
				}
				tl[i] = label_num;
			}
		}

		// 最終的に、ラベル値のセット
		for (y = 0; y < height - 1; y++)
			for (x = 0; x < width; x++)
			{
				if (work[y * pitch + x])
				{
					label1[y * pitch + x] = tl[work[y * pitch + x]];
				}
				else
				{
					label1[y * pitch + x] = 0;
				}
			}

		*label1_n = label_num;

		free(work);
		free(tl);

		return (0);
	}
	int write_toku_to_bufB(unsigned char* buf3, int* toku2, int key, int label2_n, int width, int height, int fsize, int r, int g, int b)
	{
		int label, data, x, y, i, j;
		int pitch1, pitch3;
		int img_write_num_point4_999_color(unsigned char* buf, int fsize, int x0, int y0, int width, int height, int num, int r, int g, int b);

		pitch3 = CALC_PITCH(width, 3);

		//for(j=0;j<height;j++)
		//for(i=0;i<width;i++)
		//{
		//	buf3[j*pitch3+i*3+0]=buf1[j*pitch1+i+0];
		//	buf3[j*pitch3+i*3+1]=buf1[j*pitch1+i+0];
		//	buf3[j*pitch3+i*3+2]=buf1[j*pitch1+i+0];
		//}

	//	for(label=1;label<=label2_n;label++)
		for (label = 1; label <= label2_n; label++)
		{
			data = toku2[label * DATA_N + key];
			x = toku2[label * DATA_N + 1];
			y = toku2[label * DATA_N + 2];
			img_write_num_point4_999_color(buf3, fsize, x + 10, y + 3, width, height, data, r, g, b);
		}

		return(0);
	}
	int img_write_num_point4_999_color(unsigned char* buf, int fsize, int x0, int y0, int width, int height, int num, int r, int g, int b)
	{
		int img_write_num_9_color(unsigned char* buf, int fsize, int x0, int y0, int width, int height, int num, int r, int g, int b);

		//	printf ("1, x0=%d y0=%d w=%d h=%d\n",x0,y0,width,height);

		if (num == 11111)
		{
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 0 * fsize, y0 - (8 * fsize - 1), width, height, 13, r, g, b);//1の位
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 1 * fsize, y0 - (8 * fsize - 1), width, height, 12, r, g, b);//10の位
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 2 * fsize, y0 - (8 * fsize - 1), width, height, 11, r, g, b);//10の位
		}

		else if (num == 22222)
		{
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 0 * fsize, y0 - (8 * fsize - 1), width, height, 14, r, g, b);//1の位
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 1 * fsize, y0 - (8 * fsize - 1), width, height, 11, r, g, b);//10の位
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 2 * fsize, y0 - (8 * fsize - 1), width, height, 13, r, g, b);//10の位
		}

		else
		{
			if (num > 9999) num = num % 10000;
			if (num < 0)   num = 0;

			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 0 * fsize, y0 - (8 * fsize - 1), width, height, num % 10, r, g, b);//1の位
			if (num >= 10)   img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 1 * fsize, y0 - (8 * fsize - 1), width, height, (num / 10) % 10, r, g, b);//10の位
			if (num >= 100)  img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 2 * fsize, y0 - (8 * fsize - 1), width, height, (num / 100) % 10, r, g, b);//10の位
			if (num >= 1000) img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 3 * fsize, y0 - (8 * fsize - 1), width, height, (num / 1000) % 10, r, g, b);//10の位

			//fsizeはフォントサイズで、その分、100位,10位,1位で5づつずれ、
		}

		return(0);
	}
	int img_write_num_coner4_999_color(unsigned char* buf, int fsize, int width, int height, int num, int r, int g, int b)
	{
		int img_write_num_9_color(unsigned char* buf, int fsize, int x0, int y0, int width, int height, int num, int r, int g, int b);

		img_write_num_9_color(buf, fsize, width - 1 - (5 * fsize - 1) - 5 * 0 * fsize, height - 1 - (8 * fsize - 1), width, height, num % 10, r, g, b);//1の位
		img_write_num_9_color(buf, fsize, width - 1 - (5 * fsize - 1) - 5 * 1 * fsize, height - 1 - (8 * fsize - 1), width, height, (num / 10) % 10, r, g, b);//10の位
		img_write_num_9_color(buf, fsize, width - 1 - (5 * fsize - 1) - 5 * 2 * fsize, height - 1 - (8 * fsize - 1), width, height, (num / 100) % 10, r, g, b);//10の位

		return(0);
	}
	int img_write_num_9_color(unsigned char* buf, int fsize, int x0, int y0, int width, int height, int num, int r, int g, int b)
	{
		static unsigned char numbuf[20][40] = {
			//top,,,,,, u1,,,,,,,, u2,,,,,,,, center,,,, d1,,,,,,,, d2,,,,,,,, bottom,,,, black,,,,
			{0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//0
			{0,0,1,0,0,
			 0,1,1,0,0,
			 0,0,1,0,0,
			 0,0,1,0,0,
			 0,0,1,0,0,
			 0,0,1,0,0,
			 0,1,1,1,0, 0,0,0,0,0},//1
			{0,1,1,0,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0,
			 1,0,0,0,0,
			 1,0,0,0,0,
			 0,1,1,1,0, 0,0,0,0,0},//2
			{0,1,1,0,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//3
			{0,0,1,0,0,
			 1,0,1,0,0,
			 1,0,1,0,0,
			 1,0,1,0,0,
			 1,1,1,1,0,
			 0,0,1,0,0,
			 0,0,1,0,0, 0,0,0,0,0},//4
			{0,1,1,1,0,
			 1,0,0,0,0,
			 1,0,0,0,0,
			 0,1,1,0,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//5
			{0,1,1,0,0,
			 1,0,0,0,0,
			 1,0,0,0,0,
			 1,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//6
			{1,1,1,1,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,0,1,0,0,
			 0,0,1,0,0,
			 0,0,1,0,0,
			 0,0,1,0,0, 0,0,0,0,0},//7
			{0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//8
			{0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,1,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//9
			{0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,1,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//10
			{0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 1,1,1,1,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 1,0,0,1,0, 0,0,0,0,0},//11(A)
			{0,1,1,1,0,
			 0,0,1,0,0,
			 0,0,1,0,0,
			 0,0,1,0,0,
			 0,0,1,0,0,
			 0,0,1,0,0,
			 0,1,1,1,0, 0,0,0,0,0},//12(I)
			{0,1,1,1,0,
			 1,0,0,0,0,
			 1,0,0,0,0,
			 0,1,1,0,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 1,1,1,0,0, 0,0,0,0,0},//13(S)
			{1,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 1,1,1,0,0,
			 1,0,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0, 0,0,0,0,0},//14(R)
			{0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,1,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//15
			 {0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,1,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//16
			 {0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,1,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//17
			 {0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,1,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//18
			 {0,1,1,0,0,
			 1,0,0,1,0,
			 1,0,0,1,0,
			 0,1,1,1,0,
			 0,0,0,1,0,
			 0,0,0,1,0,
			 0,1,1,0,0, 0,0,0,0,0},//19
		};
		int i, j, pitch;

		//	printf ("2, x0=%d y0=%d w=%d h=%d\n",x0,y0,width,height);

		pitch = CALC_PITCH(3, width);
		if (fsize == 0) fsize++;

		for (j = 0; j < 8 * fsize; j++)
			for (i = 0; i < 5 * fsize; i++)
				if (numbuf[num][(j / fsize) * 5 + (i / fsize)] != 0)
					if ((j + y0) <= height - 1)
						if ((j + y0) >= 0)
							if ((i + x0) <= width - 1)
								if ((i + x0) >= 0)
								{
									buf[(j + y0) * pitch + (i + x0) * 3 + 0] = r;
									buf[(j + y0) * pitch + (i + x0) * 3 + 1] = g;
									buf[(j + y0) * pitch + (i + x0) * 3 + 2] = b;
								}

		return(0);
	}
/////end //windows 9
}//linux 9