
//#include	<direct.h>				//windows 1
//#include	<opencv2/opencv.hpp>	//windows 1
#include	<sys/stat.h>			//linux 1
#include	<unistd.h>				//linux 1
#include	<opencv2/opencv.hpp>	//linux	1

using namespace cv;

////// //extern "C" {	//windows 2
extern "C" {	//linux 2 

	#define	DATA_N (100)
	#define CALC_PITCH(x,y)			(((x)*(y))%4==0) ? (x)*(y): ((((x)*(y))/4)*4+4);
	#define PL()					{}

	//windows 3
	//#define   STRCPY(a,b)			strcpy_s(a,b)
	//#define   SPRINTF               sprintf_s
	//#define	  FOPEN(fp,file,mod)	fopen_s(&fp,file,mod)
	//#define	  SSCANF				sscanf_s
	//#define	  MEMCPY				memcpy_s
	//#define   SpRINTF7(a1,a2,a3,a4,a5,a6,a7)          sprintf_s(a1,a2,a3,a4,a5,a6,a7)
	//#define   SpRINTF6(a1,a2,a3,a4,a5,a6)             sprintf_s(a1,a2,a3,a4,a5,a6)
	//linux 3
	#define STRCPY(a,b)			strcpy(a,b)
	#define SPRINTF               sprintf
	#define FOPEN(file,mod)	fopen(file,mod)
	#define SSCANF				sscanf
	#define MEMCPY				memcpy
	#define SpRINTF7(a1,a2,a3,a4,a5,a6,a7)          sprintf(a1,a3,a4,a5,a6,a7)
	#define SpRINTF6(a1,a2,a3,a4,a5,a6)             sprintf(a1,a3,a4,a5,a6)

	//opencv
	static unsigned char* Buf0;//“ü—Í‰æ‘œƒOƒ[ƒoƒ‹•Ï”

	int load_image(const char* file, int* width, int* height, int* channel, int* depth);
	int add_num_to_file(char* outfile1, char* outfile, const int cha);
	int add_char_to_file(char* outfile1, char* outfile, const char* cha);
	int calc_on_label_img1ch(unsigned long* label1, unsigned char* buf, int key, int value1, int width, int height, int* toku, int label1_n);
	int check_param(unsigned long* label0, unsigned char* buf2, int width, int height, int* toku, int label1_n, int* keys, int* mins, int* maxs, int para_n);
	int change_extension(char* outfile1, char* outfile, const char* ext);
	int check_file_exist(const char* file, int* flag);
	int get_file_extension(char* file_name, char* file_extension, char* file_plain_name);
	int get_file_name_and_dir(char* full_path, char* file_name, char* file_dir);
	int merge(unsigned short* buf3, unsigned char* buf2, unsigned long* label0, int* toku, int width, int height);
	int read_src_param(char* parafile, int* posi_nega, int* rate, int* step, int* evkey, int* evflag, int* keys, int* mins, int* maxs, int* para_n);
	int report_toku30(char* file, int* toku, int label1_n);
	int Src_label2L(unsigned long* label1, unsigned char* buf74, int width, int height, int* label1_n);
	int save_image(char* file0, unsigned char* buf, int width, int height, int channel, int depth);


	int main(int argc, char* argv[])
	{
		char app[512], infile[512], outfile[512], name[512], dir[512], work[512], outfile0[512];
		char parafile[512];
		int kando;

		int seg(char* infile, char* outfile, char* parafile, char* work, int kando);

		if (argc >= 1) STRCPY(app, argv[0]);
		if (argc >= 2) STRCPY(infile, argv[1]);
		if (argc >= 3) STRCPY(outfile0, argv[2]);
		if (argc >= 4) STRCPY(parafile, argv[3]);
		if (argc >= 5) kando = atoi(argv[4]);
		//if (kando == 0) kando = 1;

		//out-pathŽæ“¾
		get_file_name_and_dir(outfile0, name, dir);

		//out-path‚Ì’¼‰º‚É work=tempì¬BiƒfƒoƒbƒO—p‚Ìƒtƒ@ƒCƒ‹“™‚ð•Û‘¶)
		SPRINTF(work, "%s/temp", dir);

		//_mkdir(work);		//windows 4
		mkdir(work, 0777);//linux 4

		//outfile‚ÌŠg’£Žq‚ð•ÏX(jpg)
		change_extension(outfile, outfile0, ".jpg");

		//ƒfƒoƒbƒO—p‚Ì•\Ž¦(Á‚·‚È)
		printf("infile   =(%s)\n", infile);
		printf("outfile  =(%s)\n", outfile);
		printf("parafile =(%s)\n", parafile);
		printf("work     =(%s)\n", work);
		printf("kando    =(%d)\n", kando);

		seg(infile, outfile, parafile, work, kando);

		return(0);
	}
	int seg(char* infile, char* outfile, char* parafile, char* work, int Kando)
	{
		char file[512], file1[512], file2[512];
		int width, height, channel, depth;
		int src_flag, src_rate, src_step, src_evkey, src_evtype, nof_keys, src_ume;
		int keys[30], mins[30], maxs[30];
		int i, j, pitch1, pitch3;
		unsigned char* buf, * buf2, * buf2a;
		unsigned short* bufm;
		unsigned long* label1;
		int label1_n, th, * toku;
		int debug_flag0 = 0;
		int kousi = 0;
		int param[5];

		int Src_label3L(unsigned long* label1, unsigned char* buf74, int width, int height, int* label1_n);
		int save_label2(char* file1, char* file2, int* toku, int label1n, unsigned long* buf, int width, int height, int channel);
		int save_image_down2(char* file, unsigned short* bufm, int width, int height);
		int img_choose_white(unsigned char* buf1, int min, int width, int height);
		int read_src_paramB(char* parafile, int* param, int* posi_nega, int* rate, int* step, int* evkey, int* evflag, int* keys, int* mins, int* maxs, int* para_n);
		int img_ave(unsigned char* buf2, unsigned char* buf1, int width, int height, int full_sizea, int channel, int depth);
		int img_max(unsigned char* buf2, unsigned char* buf1, int width, int height, int full_sizea, int channel, int depth);
		int img_min(unsigned char* buf2, unsigned char* buf1, int width, int height, int full_sizea, int channel, int depth);

		//‰æ‘œ
		load_image(infile, &width, &height, &channel, &depth);
		printf("w=%d h=%d ch=%d d=%d\n", width, height, channel, depth);

		//add_num_to_file(file,outfile,100);
		add_num_to_file(file, outfile, 100);
		save_image(file, Buf0, width, height, channel, 1);

		//ƒpƒ‰ƒ[ƒ^ƒtƒ@ƒCƒ‹“Ç‚Ýž‚Ý
		read_src_paramB(parafile, param, &src_flag, &src_rate, &src_step, &src_evkey, &src_evtype, keys, mins, maxs, &nof_keys); //ƒpƒ‰ƒ[ƒ^“Ç‚Ýž‚Ý

		printf("%d %d %d %d %d %d\n", src_flag, src_rate, src_step, src_evkey, src_evtype, nof_keys);
		mins[1] = mins[1] * (100 - Kando) / 50;
		for (i = 0; i < nof_keys; i++)
			printf("key=%d (%d,%d)\n", keys[i], mins[i], maxs[i]);

		//src-flagF   ƒ|ƒW‚©ƒlƒK‚©(0:nega,1:posi)
		//src_rateF   k¬—¦A¡‰ñ‚ÍŽg‚Á‚Ä‚¢‚È‚¢B
		//src_stepF   ‚Q’l‰»‚Ì‚Ý‚ÌŠÔŠu(ŒvŽZŽžŠÔÌ¸“xj
		//src_evkeyF  •]‰¿ŠÖ”‚É—p‚¢‚é“Á’¥—Ê
		//src_evtypeF •]‰¿•û–@(‚‚¢E’á‚¢E’l‚É‹ß‚¢)
		//key,min,maxF—Ìˆæ‘I•Ê‚Ìˆ×‚ÌA“Á’¥—Ê‚Æ‚»‚ÌãŒÀA‰ºŒÀ
		//nof_keysF@ “Á’¥—Ê‚Ì”

//ˆ—ƒtƒ‰ƒO
		if (param[0] == 0);//‰½‚à‚µ‚È‚¢
		else if (param[0] == 1);//minmax
		else;//maxmin

		src_ume = 0;
		if (src_flag >= 2)
		{
			printf("src_flag=%d \n", src_flag);
			src_ume = src_flag - (src_flag % 2);
			src_flag = (src_flag % 2);
			printf("src_flag=%d src_ume=%d\n", src_flag, src_ume);
		}
		if (src_flag < 0)
		{
			kousi = abs(src_flag);//ŠiŽqŠÔŠu
			src_flag = abs(src_flag) % 2;
			debug_flag0 = 1;
		}
		//printf("src_flag=%d src_ume=%d\n", src_flag, src_ume);

		pitch1 = CALC_PITCH(width, 1);
		pitch3 = CALC_PITCH(width, 3);

		buf = (unsigned char*)calloc(height * pitch1, 1);//ƒOƒŒ[ƒXƒP[ƒ‹‰»‚µ‚½Œ³‰æ‘œ
		buf2 = (unsigned char*)calloc(height * pitch1, 1);//‰ÁH‰æ‘œ
		buf2a = (unsigned char*)calloc(height * pitch1, 1);//‰ÁH‰æ‘œ
		bufm = (unsigned short*)calloc(2, pitch1 * height);

		long long sum[4];
		int flag[4];//ŠeF‚ª‘S•”0‚©255‚©‚Ìƒtƒ‰ƒO
		int k, nof_flag, ss, max0;

		//buf‚Í’Šo‘ÎÛ‰æ‘œ
		if (channel != 1)//1F‚Å‚È‚¢ê‡
		{
			//3,4F’†1FƒOƒŒ[‚©‚Ç‚¤‚©‚Ì”»’è
			sum[0] = sum[1] = sum[2] = sum[3] = 0;
			flag[0] = flag[1] = flag[2] = flag[3] = 0;//ŠeF‚Ìƒ`ƒƒƒlƒ‹‚ª—LŒø‚©‚Ìƒtƒ‰ƒO
			for (k = 0; k < channel; k++)
			{
				for (j = 0; (j < height) && (flag[k] == 0); j++)
					for (i = 0; i < width; i++)
					{
						sum[k] += Buf0[j * pitch3 + i * channel + k];
						//if ((j * width + i)<200) //Šm”F—p
						//{printf("k=%d n=%d sum=%d\n", k, (j* width + i), sum[k]);
						//}
						if ((sum[k] != 0) && (sum[k] != (j * width + i) * 255))//‘S•”‚ª0‚©255‚Å‚È‚¢
						{
							flag[k] = 1; break;
						}
					}
			}
			nof_flag = 0;
			for (k = 0; k < channel; k++)
				if (flag[k] != 0) nof_flag++;//—LŒø‚Èƒ`ƒƒƒlƒ‹”

				//printf("nof_flag=%d channel=%d \n", nof_flag, channel);

				//—LŒø‚Èƒ`ƒƒƒ“ƒlƒ‹‚Ì‚Ý‚Ì•½‹Ï‚Æ‚·‚é
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

		//ƒOƒŒ[ƒXƒP[ƒ‹‰»‚ÌŒ‹‰ÊŠm”F
		//add_num_to_file(file, outfile, 101);
		//save_image(file, buf, width, height, 1, 1);

		if (src_flag == 0)//”½“](ˆÃ‚¢•û‚ðƒT[ƒ`‚·‚éê‡)0:nega 1:posi
			for (i = 0; i < height * pitch1; i++) buf[i] = 255 - buf[i];

		//ƒOƒŒ[ƒXƒP[ƒ‹Šm”F‰æ‘œ(”½“]—L–³ž‚Ý)
		//add_num_to_file(file,outfile,102);
		//save_image(file, buf,width,height,1,1) ;

//‘Oˆ—‚ÌŽÀŽ{
		if (param[0] == -1)//minmax
		{
			img_min(buf2, buf, width, height, param[1], 1, 1);
			memcpy(buf, buf2, height * pitch1);
		}
		else if (param[0] == -2)//minmax
		{
			img_min(buf2, buf, width, height, param[1], 1, 1);
			img_max(buf, buf2, width, height, param[1], 1, 1);
		}
		else if (param[0] == 1)
		{
			img_max(buf2, buf, width, height, param[1], 1, 1);
			memcpy(buf, buf2, height * pitch1);
		}
		else if (param[0] == 2)
		{
			img_max(buf2, buf, width, height, param[1], 1, 1);
			img_min(buf, buf2, width, height, param[1], 1, 1);
		}
		else;

		if (param[2] != 0)
		{
			img_ave(buf2, buf, width, height, param[2], 1, 1);
			memcpy(buf, buf2, height * pitch1);
		}

		//ƒOƒŒ[ƒXƒP[ƒ‹Šm”F‰æ‘œ(”½“]—L–³ž‚Ý)
		add_num_to_file(file, outfile, 102);
		save_image(file, buf, width, height, 1, 1);

		//Å‘å’l‚Å‹KŠi‰»
		max0 = 0;//Å‘å’l
		for (i = 0; i < height * pitch1; i++)
			if (buf[i] > max0) max0 = buf[i];

		if (max0 != 0)//‹KŠi‰»
			for (i = 0; i < height * pitch1; i++)
				buf[i] = buf[i] * 255 / max0;

		if (kousi != 0)
			for (i = 0; i < height * pitch1; i++)
			{
				if ((i % pitch1) % kousi == 0) { buf[i] = 0; }
				if ((i / pitch1) % kousi == 0) { buf[i] = 0; }
			}

		//ƒOƒŒ[ƒXƒP[ƒ‹Šm”F‰æ‘œ(”½“]—L–³ž‚Ý)
		add_num_to_file(file, outfile, 103);
		save_image(file, buf, width, height, 1, 1);

		if (kousi != 0)
			for (i = 0; i < height * pitch1; i++)
			{
				buf[i] = 255 - buf[i];
			}

		//ƒOƒŒ[ƒXƒP[ƒ‹Šm”F‰æ‘œ(”½“]—L–³ž‚Ý)
		add_num_to_file(file, outfile, 104);
		save_image(file, buf, width, height, 1, 1);

		if (kousi != 0)//–ß‚·B
			for (i = 0; i < height * pitch1; i++)
			{
				buf[i] = 255 - buf[i];
			}

		label1 = (unsigned long*)calloc(width * height, sizeof(unsigned long));

		//ƒfƒoƒbƒOŽž‚É“r’†Œo‰ß‚ðŒ©‚½‚¯‚ê‚Îdebug_flag0‚ð1‚É‚·‚éB
		int count, count_old;

		//ƒƒCƒ“ƒ‹[ƒv
		count = 0;
		for (th = 255; th >= 0; th -= src_step)//—Ìˆæ‚ð™X‚ÉL‚°‚é
		{
			//2’l‰»(buf2<-buf)
			count_old = count;
			count = 0;
			for (i = 0; i < height * pitch1; i++)
			{
				if (buf[i] >= th) { buf2[i] = 255; count++; }
				else            buf2[i] = 0;
			}
			if (count == count_old) continue;//‘O‚Ì“ñ’l‰»‚Æ“¯‚¶‚È‚ç‚â‚ç‚È‚¢B
			if (count == height * pitch1) continue;
			if (count == 0) continue;
			printf("th=%d count=%d %d\n", th, count, height * pitch1);//Œ»Ý‚Ìè‡’l

			if (debug_flag0 != 0)
			{
				SPRINTF(file, "%s/_0_%3d.jpg", work, th);
				save_image(file, buf2, width, height, 1, 1);
			}

			if (src_ume != 0)//–„‚ßž‚Ý‚ð‚¨‚±‚È‚¤ê‡B
			{
				memcpy(buf2a, buf2, pitch1 * height);
				//SPRINTF(file, "%s\\_91.jpg", work);
				//save_image(file, buf2a, width, height, 1, 1);
				for (i = 0; i < pitch1 * height; i++) buf2a[i] = 255 - buf2a[i];//”½“]
							//SPRINTF(file, "%s\\_92.jpg", work);
							//save_image(file, buf2a, width, height, 1, 1);
				img_choose_white(buf2a, src_ume, width, height);
				//SPRINTF(file, "%s\\_93.jpg", work);
				//save_image(file, buf2a, width, height, 1, 1);
				for (i = 0; i < pitch1 * height; i++) buf2[i] = buf2[i] + buf2a[i];//—Ìˆæ‡¬
							//SPRINTF(file, "%s\\_94.jpg", work);
							//save_image(file, buf2a, width, height, 1, 1);
			}

			Src_label3L(label1, buf2, width, height, &label1_n);//1

			toku = (int*)calloc(4, (label1_n + 1) * DATA_N);

			//ƒ‰ƒxƒŠƒ“ƒOŒ‹‰Ê‚Ì•\Ž¦
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
			//ðŒ‚ð–ž‚½‚³‚È‚¢—Ìˆæ‚ðbuf2‚©‚ç‚Í‚¸‚·

			if (debug_flag0 != 0)
			{
				SPRINTF(file, "%s/_5.jpg", work);
				save_image(file, buf2, width, height, 1, 1);
				SPRINTF(file, "%s/_6_%3d.csv", work, th);
				report_toku30(file, toku, label1_n);
			}

			merge(bufm, buf2, label1, toku, width, height);
			//bufm‚Ì‹L˜^‚ð’´‚¦‚élabel‚ª‚ ‚ê‚ÎA‹L˜^‚Ælabel(buf2)‚ðAbufm‚©‚çXV‚·‚é

			if (debug_flag0 != 0)
			{
				SPRINTF(file, "%s/_7_%3d.jpg", work, th);
				save_image(file, buf2, width, height, 1, 1);
				SPRINTF(file, "%s/_8_%3d.jpg", work, th);
				save_image_down2(file, bufm, width, height);
			}

			free(toku);
		}

		//ÅIŒ‹‰Ê(bufm)‚ð‚Q’l‰»
		for (i = 0; i < height * pitch1; i++)
			if (bufm[i] != 0) buf2[i] = 255;
			else            buf2[i] = 0;

		unsigned char* buf3, *buf4;
		int data, la, r, g, b;
		int rtbl[256], gtbl[256], btbl[256];

		buf3 = (unsigned char*)calloc(height * pitch1, 1);//‰ÁH‰æ‘œ
		buf4 = (unsigned char*)calloc(height * pitch3, 1);//‰ÁH‰æ‘œ

					//Ä‰ðÍ
		Src_label3L(label1, buf2, width, height, &label1_n);//1

		toku = (int*)calloc(4, (label1_n + 1) * DATA_N);
		calc_on_label_img1ch(label1, buf, src_evkey, src_evtype, width, height, toku, label1_n);

		//‹[Ž—ƒJƒ‰[•\Ž¦—pƒe[ƒuƒ‹
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

		//Še“Á’¥—Ê‰ðÍ‚Ì}
		for (k = 0; k < nof_keys; k++)
		{
			for (j = 0; j < height; j++)
				for (i = 0; i < width; i++)
				{
					la = label1[j * width + i];//ƒ‰ƒxƒ‹Žæ“¾
					if (la == 0)
					{
						buf3[j * pitch1 + i] = 0;//data->‹KŠi‰»->rgb),buf4‚É‹L˜^
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

					buf3[j * pitch1 + i] = data;//data->‹KŠi‰»->rgb),buf4‚É‹L˜^

					buf4[j * pitch3 + i * 3 + 0] = rtbl[data];
					buf4[j * pitch3 + i * 3 + 1] = gtbl[data];
					buf4[j * pitch3 + i * 3 + 2] = btbl[data];
				}

			//“Á’¥—Ê‚Ì‹L˜^
			//add_num_to_file(file, outfile, 210+k);
			//save_image(file, buf3, width, height, 1, 1);
			add_num_to_file(file, outfile, 250 + k);
			save_image(file, buf4, width, height, 3, 1);
		}
		//“Á’¥—Ê‚Ì‹L˜^
		add_num_to_file(file, outfile, 300);
		change_extension(file1, file, ".csv");
		report_toku30(file1, toku, label1_n);

		//ƒ‰ƒxƒŠƒ“ƒOŒ‹‰Ê‚Ì•\Ž¦
		add_char_to_file(file1, outfile, "a");
		add_char_to_file(file2, outfile, "b");
		save_label2(file1, file2, toku, label1_n, label1, width, height, channel);

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
	int save_image_down2(char* file, unsigned short* bufm, int width, int height)
	{
		int i, pitch, a;
		unsigned char* buf2;
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
			a = (int)(bufm[i] * 64 / (ss / 2));
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
		check_file_exist(file, &flag);//ƒtƒ@ƒCƒ‹‚Ì—L–³Šm”F
		{
			if (flag == 0) return(1);//ƒtƒ@ƒCƒ‹‚ª‚È‚¢ƒGƒ‰[
		}

		width0 = image.size().width;
		height0 = image.size().height;
		depth0 = image.depth();//CV_8U,CV_16U‚©
		channel0 = image.channels();

		if (depth0 == CV_8U) { depth0 = 1; *depth = 1; }//‘S•Ï”‚Ì’è‹`
		else if (depth0 == CV_16U) { depth0 = 2; *depth = 2; }
		*width = width0;
		*height = height0;
		*channel = channel0;
		pitch0 = CALC_PITCH(width0, channel0 * depth0);

		if (*depth == 1)
			Buf0 = (unsigned char*)calloc(sizeof(unsigned char), pitch0 * height0);
		if (*depth == 2)
			Buf0 = (unsigned char*)calloc(sizeof(unsigned char), pitch0 * height0);

		//ƒtƒ@ƒCƒ‹‚ÌƒTƒCƒYŠm”F
		printf("%d %d %d %d %d\n", width0, height0, channel0, depth0, pitch0);

		//Buf0‚Éƒf[ƒ^‚ðˆÚ‚µ‘Ö‚¦‚é

		if (depth0 == 1)
			for (j = 0; j < height0; j++)//image.data‚Íraw‚Ì‚×‚½‘‚«
				for (i = 0; i < width0; i++)
					for (k = 0; k < channel0; k++)
						Buf0[j * pitch0 + i * channel0 + k] = image.data[(j * width0 + i) * channel0 + k];

		if (depth0 == 2)
			for (j = 0; j < height0; j++)//image.data‚Íraw‚Ì‚×‚½‘‚«
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

		if (depth == 2)//tifŒ`Ž®‚Å‚È‚¢ê‡
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
		else if ((depth == 4) && (channel == 1)) bit_type = CV_8UC1;//ƒ‰ƒxƒ‹•\Ž¦

		Mat image(Size(width, height), bit_type, buf2);

		pitch = CALC_PITCH(width, channel * depth);

		//	printf ("save:(%s)(w=%d H=%d depth=%d channel=%d)\n",file,image.size().width,image.size().height,
		//		image.depth(),image.channels());

			//“ü‚ê‘Ö‚¦
		if (depth == 1)
			for (j = 0; j < height; j++)
				for (i = 0; i < width; i++)
					for (k = 0; k < channel; k++)
					{
						//		printf ("%d %d\n",(j*width+i)*channel+k,image.data[(j*width+i)*channel+k]);
						image.data[(j * width + i) * channel + k] = buf[j * pitch + i * channel + k];
					}

		//“ü‚ê‘Ö‚¦
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
	int get_file_extension(char* file_name, char* file_extension, char* file_plain_name)
	{
		int i, len, p;

		len = (int)strlen(file_name);

		//Œã‚ë‚©‚çƒT[ƒ`‚µ‚Ä'.'‚ð’T‚·B
		for (i = len - 1; i >= 0; i--)
			if (file_name[i] == '.') break;

		p = i;

		//file_plain_name‚ÉŠÖ‚µ‚Ä‚Íp”Ô–Ú‚Ü‚Å‚ðƒRƒs[‚µ‚ÄAÅŒã‚ðI’[B
		//file_extension‚ÉŠÖ‚µ‚Ä‚Íp”Ô–ÚˆÈ~‚ðƒRƒs[‚µ‚ÄAÅŒã‚ðI’[B

		for (i = 0; i < p; i++)
			file_plain_name[i] = file_name[i];

		//printf("1(%s)\n", file_plain_name);

		file_plain_name[p] = 0;//‚±‚±‚Ífor•¶‚ÌŠOAŽŸ‚Ì—v‘f‚ðƒ[ƒ‚É‚·‚éBiI’[)

	//	printf("\t1(%s)\n", file_plain_name);

		for (i = p; i < len; i++)
			file_extension[i - p] = file_name[i];
		file_extension[i - p] = 0;//‚±‚±‚Ífor•¶‚ÌŠOAŽŸ‚Ì—v‘f‚ðƒ[ƒ‚É‚·‚éB(I’[)
	//	file_extension[i-p+1]=0;//‚±‚±‚Ífor•¶‚ÌŠOAŽŸ‚Ì—v‘f‚ðƒ[ƒ‚É‚·‚éB(I’[)

		return(0);
	}
	int check_file_exist(const char* file, int* flag)//ƒtƒ@ƒCƒ‹ƒ`ƒFƒbƒN‚µ‚È‚¢‚Æexit‚·‚é
	{
		FILE* fp;

		//FOPEN(fp,file, "rb");//windows 5
		fp = FOPEN(file, "rb");//linux 5

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

		for (i = 1; i <= label1_n; i++)//‘Ê–Ú‚È‚à‚Ì‚ðƒ`ƒFƒbƒN‚·‚é
		{
			for (k = 0; k < para_n; k++)
			{
				if ((keys[k] < 0) || (keys[k] >= DATA_N)) continue;
				v = toku[i * DATA_N + keys[k]];
				if ((v >= mins[k]) && (v <= maxs[k])) {}
				else {
					toku[i * DATA_N + DATA_N - 1] = -1;
					toku[i * DATA_N + DATA_N - 5] = keys[k];//DATA_N-3,DATA_N-2”Ô‚É‚Í“K‚³‚È‚¢——R‚ð‘‚­
					toku[i * DATA_N + DATA_N - 4] = mins[k];//DATA_N-3,DATA_N-2”Ô‚É‚Í“K‚³‚È‚¢——R‚ð‘‚­
					toku[i * DATA_N + DATA_N - 3] = maxs[k];
					break;
				}//1ŒÂ‚Å‚àðŒ‚ð–ž‚½‚³‚È‚¯‚ê‚Î”²‚¯‚éB
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

		//label‚ª0‚Å‚È‚¢‚à‚Ì‚ÌAlab‚Ætoku->buf3‚Ìev‚Æ”äŠr‚µA‹­‚³”äŠr

		for (j = 0; j < height; j++)
			for (i = 0; i < width; i++)
			{
				lab = label0[j * width + i];
				if (lab == 0) continue;
				if (toku[lab * DATA_N + DATA_N - 1] == -1) {}//–³Œø
				else//—LŒø
				{
					ev = toku[lab * DATA_N + DATA_N - 2];//•]‰¿’l
					if (ev < buf3[j * pitch + i]) toku[lab * DATA_N + DATA_N - 1] = -1;//ƒtƒ‰ƒO‚ð—Ž‚Æ‚·(1‚Â‚Å‚à•‰‚¯‚ê‚Î)
				}
			}

		for (j = 0; j < height; j++)
			for (i = 0; i < width; i++)
			{
				lab = label0[j * width + i];
				if (lab == 0) continue;
				if (toku[lab * DATA_N + DATA_N - 1] == -1) {}//–³Œø
				else
				{
					ev = toku[lab * DATA_N + DATA_N - 2];//•]‰¿’l
					buf3[j * pitch + i] = ev;//ƒtƒ‰ƒO‚ð‘‚«ž‚Þ(ƒ}[ƒW‚µ‚½‚Æ‚¢‚¤ˆÓ–¡)
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
		//•]‰¿ŠÖ”—p
		// 
		//	toku[80] = ‘å‚«‚³   * ‰~Œ`“x;
		//	toku[81] = ‰~Œ`“x   * toku[65](=ave-ŠO)
		//	toku[82] = toku[65] * ‘å‚«‚³
		//	toku[83] = ‰~Œ`“x @* toku[63](=max-min)
		//	toku[84] = toku[63] * ‘å‚«‚³
		//
		//	toku[85] = ‘å‚«‚³   * (10000-‰~Œ`“x);
		//	toku[86] = (10000 - ‰~Œ`“x) * toku[65])(ave-ŠO)
		//
		//¡–@Œn
		//			toku[0] = 0;//ƒJƒEƒ“ƒ^
		//			toku[1] +=i;//xdS
		//			toku[2] +=j;//ydS
		// 	   		toku[3] = xmin;
		//			toku[4] = xmax;
		//			toku[5] = ymin;
		//			toku[6] = ymax;
		//			toku[7] = width0;
		//			toku[8] = height0;
		// 	   		toku[9] = len[0]; //0“x•ûŒü’·‚³
		//			toku[9+i]=len[i]; //i*11“x•ûŒü’·‚³
		//			toku[24]= len[15];//15*11•ûŒü’·‚³
		// 	        toku[25];//ƒJƒEƒ“ƒg‚Ì10000•ª—¦
		// 	        toku[25+i];//“Á’¥—Êi‚Ì10000•ª—¦
		//	        toku[33+i];//height0‚Ì10000•ª—¦
		//34-38:Œ‡”Ô
		// 	   		toku[39] = maxL*10000/minL;//‹tG•½—¦
		//			toku[40] = minL;//’Z•Ó
		//			toku[41] = maxL;//’·•Ó
		//			toku[42] = minL * 10000 / maxL;//G•½—¦
		// 
		// 	   		toku[43] =count*1000.0/(maxL*maxL*3.14159/2.0/2.0)+0.5);//‰~Œ`“x
		//			toku[44] =count*1000.0/(minL*maxL*3.14159/2.0/2.0)+0.5);//‘È‰~“x
		// 			toku[45] =round;//ŽlŠp“x
		//			toku[46] =round;//ŠOŽü(63‚Æ“¯‚¶j
		//			toku[47] =count*2/(round);//•½‹Ï‘¾‚³
		//			toku[48] =count/maxL;//×’·‚³’·‚³/‘¾‚³
		//			toku[49] =round*1000/(minL+maxL)/2;//Ü‚è•Ô‚µ”
		//			toku[50] =mindeg;//Œü‚«(“x)
		//			toku[51] =count/round;//ŠOŽü–ÊÏ”ä
		//—Ìˆæ(w,h):d•¡
		//			toku[53] x0
		//			toku[54] x1
		//			toku[55] y0
		//			toku[56] y1
		//			toku[57] = toku[i * DATA_N + 54] - toku[i * DATA_N + 53] + 1;//width0
		//			toku[58] = toku[i * DATA_N + 56] - toku[i * DATA_N + 55] + 1;//height0
		//’lŒn
		//			toku[60] buf‚Ì’l;//•½‹Ï’l
		//			toku[61] min@buf‚Ì’l
		//			toku[62] max@buf‚Ì’l
		// 			toku[63] =maxbuf-minbuf //“à•”ƒRƒ“ƒgƒ‰ƒXƒgi“à•”‚ÌÅ‘å[“à•”‚ÌÅ¬j
		//ƒRƒ“ƒgƒ‰ƒXƒgŒn
		//			toku[64]/=toku[i * DATA_N +63];//’¼ŠO‰æ‘f‚Ì•½‹Ï’l
		// 			toku[65]=toku[i * DATA_N +60]-toku[i * DATA_N +64];//“àŠOƒRƒ“ƒgƒ‰ƒXƒg(’†•½‹Ï‚Æ’¼ŠO‘¤‚Ì·j
		//			toku[66]=toku[i * DATA_N +61]-toku[i * DATA_N +64];//“àŠOƒRƒ“ƒgƒ‰ƒXƒgi’†Å¬’l‚Æ’¼ŠO’l)
		//			toku[74] = i;//ƒ‰ƒxƒ‹”Ô†
		//•]‰¿ŠÖ”—p
		//			toku[80] = toku[ 0]*toku[43]
		// 			toku[81] = toku[43]*toku[65]
		// 			toku[82] = toku[65]*toku[ 0]
		//§Œä•Ï”
		//		toku[DATA_N-5]=key;•s‡Ši——R //95
		//		toku[DATA_N-4]=min0;•s‡Ši——R //96
		//		toku[DATA_N-3]=max0;•s‡Ši——R //97
		//		toku[DATA_N-2]=65535-abs(toku[i * DATA_N +key]-value);•]‰¿ŠÖ” //98
		//		toku[DATA_N-1]=0,-1;—ÌˆæŒ‹‡ƒtƒ‰ƒO //99

		//‰æ‘f’l‚ðŠÜ‚Þ“Á’¥—Ê
		int i, j, la, pitch, count, width0, height0;
		int minL, maxL, round, v;
		int deg, len[16], xmin, xmax, ymin, ymax, mindeg, maxdeg;
		double sin0[20], cos0[20];
		double en;
		int daen, daen0;

		int set_sin_cos(double* sin, double* cos);

		pitch = CALC_PITCH(1, width);

		set_sin_cos(sin0, cos0);//ŽOŠpŠÖ”•\

		//‰Šú‰»
		for (i = 1; i <= label1_n; i++)
		{
			for (j = 0; j < DATA_N; j++) toku[i * DATA_N + j] = 0;
			toku[i * DATA_N + 53] = width - 1;//Å¬’l
			toku[i * DATA_N + 55] = height - 1;//Å¬’l

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

				toku[la * DATA_N + 0]++;//ƒJƒEƒ“ƒ^‰ÁŽZ(Å‰”‚¾‚¯”‚¦‚é)

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

				for (deg = 0; deg < 16; deg++)//“Š‰e’·(min)
					if (len[deg] < toku[la * DATA_N + 3 + deg * 2]) { toku[la * DATA_N + 3 + deg * 2] = len[deg]; }

				for (deg = 0; deg < 16; deg++)//“Š‰e’·(max)
					if (len[deg] > toku[la * DATA_N + 4 + deg * 2]) { toku[la * DATA_N + 4 + deg * 2] = len[deg]; }

				v = buf[j * pitch + i];//ƒOƒŒ[ƒXƒP[ƒ‹‚Ì”’l
				if (toku[la * DATA_N + 60] < 2000000000) toku[la * DATA_N + 60] += v;//”’l

				if (v < toku[la * DATA_N + 61]) toku[la * DATA_N + 61] = v;//min
				if (v > toku[la * DATA_N + 62]) toku[la * DATA_N + 62] = v;//max

				//‹ß—×‚É•Ê‚Ìƒ‰ƒxƒ‹‚ª‚ ‚ê‚Î(8•ûŒü)
				if ((i != 0) && (j != 0) && (i != width - 1) && (j != height - 1))
				{
					//‹ß—×‚É•Ê‚Ìƒ‰ƒxƒ‹‚ª‚ ‚ê‚Î(4•ûŒü),ŠO‘¤‚Ì“_‚Ì’l‰ÁŽZ,63‚ÍŒÂ”,64‚Í”’l,
					if (label1[(j - 1) * width + (i)] == 0) { toku[la * DATA_N + 63]++; toku[la * DATA_N + 64] += buf[(j - 1) * pitch + (i)]; }
					if (label1[(j)* width + (i - 1)] == 0) { toku[la * DATA_N + 63]++; toku[la * DATA_N + 64] += buf[(j)* pitch + (i - 1)]; }
					if (label1[(j)* width + (i + 1)] == 0) { toku[la * DATA_N + 63]++; toku[la * DATA_N + 64] += buf[(j)* pitch + (i + 1)]; }
					if (label1[(j + 1) * width + (i)] == 0) { toku[la * DATA_N + 63]++; toku[la * DATA_N + 64] += buf[(j + 1) * pitch + (i)]; }
				}
			}

		for (i = 1; i <= label1_n; i++)
		{
			count = toku[i * DATA_N + 0];
			round = toku[i * DATA_N + 63];//ŽüˆÍ’·
			if (count == 0) count++;
			if (round == 0) round++;

			toku[i * DATA_N + 1] = toku[i * DATA_N + 1] / count;//dS
			toku[i * DATA_N + 2] = toku[i * DATA_N + 2] / count;//dS

			for (deg = 0; deg < 16; deg++) //16•ûˆÊ’·‚ð‘Þ”ð
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

			for (deg = 0; deg < 16; deg++) //16•ûˆÊ’·
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

			minL = toku[i * DATA_N + 9];//’Z•Ó
			mindeg = 0;
			maxdeg = 90;
			for (deg = 0; deg < 16; deg++)
				if (toku[i * DATA_N + 9 + deg] < minL)
				{
					minL = toku[i * DATA_N + 9 + deg];
					mindeg = (int)((double)deg * 11.25 + 0.5);
				}
			if ((mindeg == 0) && (width0 == height0)) mindeg = 180;

			maxL = toku[i * DATA_N + 9];//’Z•Ó
			for (deg = 0; deg < 16; deg++)
				if (toku[i * DATA_N + 9 + deg] > maxL)
				{
					maxL = toku[i * DATA_N + 9 + deg];
					maxdeg = (int)((double)deg * 11.25 + 0.5);
				}

			if (minL == 0) minL++;//ƒ[ƒŠ„‘Îô
			if (maxL == 0) maxL++;

			toku[i * DATA_N + 39] = maxL * 10000 / minL;//‹tG•½—¦
			toku[i * DATA_N + 40] = minL;//’Z•Ó
			toku[i * DATA_N + 41] = maxL;//’·•Ó
			toku[i * DATA_N + 42] = minL * 10000 / maxL;//G•½—¦

			//‰~Œ`“x(maxLŠî€[“U“x)
			en = (double)maxL * (double)maxL * (3.14159 / 2.0 / 2.0);
			toku[i * DATA_N + 43] = (int)(((double)((double)count / en) * 10000.0) + 0.5);//‰~Œ`“x
			if (toku[i * DATA_N + 43] > 10000) toku[i * DATA_N + 43] = 10000;
			if (count == 1) toku[i * DATA_N + 43] = 1;//‘å‚«‚³1‚Å‰~Œ`“x‚ªmax‚È‚ç‚È‚¢‚æ‚¤‚É
			//Å’·•Ó‚ð’¼Œa‚Æ‚·‚é‰~‚ÆA–ÊÏ‚Ì”ä(‰~‚È‚ç10000‚»‚êˆÈŠO‚ÍA¬‚³‚¢”Žš)

			daen = 0;//‘È‰~“x
			for (deg = 0; deg < 8; deg++)
			{
				daen0 = toku[i * DATA_N + 9 + deg] * toku[i * DATA_N + 17 + deg];
				daen0 = (int)(((double)((double)count / (double)daen0 / (3.14159 / 2.0 / 2.0)) * 10000.0) + 0.5);//‘È‰~“x
				if (daen0 > daen) daen = daen0;
			}
			toku[i * DATA_N + 44] = daen;
			//‘È‰~‚É‘Î‚·‚éA–ÊÏ‚Ì”ä(‰~‚È‚ç10000‚»‚êˆÈŠO‚ÍA¬‚³‚¢”Žš)

			toku[i * DATA_N + 45] = (int)((double)count * 10000.0 / (double)(width0 * height0));//ŽlŠp“x

			toku[i * DATA_N + 46] = round;//ŠOŽü‚ð•Û‘¶
			toku[i * DATA_N + 47] = (int)(((double)count * 2.0 / (double)round + 0.5) * 10.0);//‘¾‚³
			if (toku[i * DATA_N + 47] == 0) toku[i * DATA_N + 47]++;
			toku[i * DATA_N + 48] = (int)((double)(round * 10.0) / ((double)(toku[i * DATA_N + 47]) * 2.0) + 0.5);//×’·‚³
			if (toku[i * DATA_N + 47] == 0) toku[i * DATA_N + 47] = 1;
			if (toku[i * DATA_N + 48] == 0) toku[i * DATA_N + 48] = 1;

			toku[i * DATA_N + 49] = round * 10 / ((minL + maxL) * 2);//Ü‚è•Ô‚µ‰ñ”
			if (toku[i * DATA_N + 49] > 10000)toku[i * DATA_N + 49] = 10000;

			toku[i * DATA_N + 50] = (count * 10) / round;//ŠOŽü–ÊÏ”ä
			toku[i * DATA_N + 51] = mindeg;//’Z•Ó•ûˆÊŠp
			toku[i * DATA_N + 52] = maxdeg;//’·•Ó•ûˆÊŠp

			toku[i * DATA_N + 60] /= count;//•½‹Ï’l
			//61 min
			//62 max
			toku[i * DATA_N + 63] = toku[i * DATA_N + 62] - toku[i * DATA_N + 61];//con_inner“à•”·
			toku[i * DATA_N + 64] /= round;//ŠO•½‹Ï
			toku[i * DATA_N + 65] = toku[i * DATA_N + 60] - toku[i * DATA_N + 64];//“àŠOƒRƒ“ƒgƒ‰ƒXƒg(ave-ŠOj
			toku[i * DATA_N + 66] = toku[i * DATA_N + 61] - toku[i * DATA_N + 64];//“àŠOƒRƒ“ƒgƒ‰ƒXƒg(min-ŠO)

			toku[i * DATA_N + 74] = i;

			toku[i * DATA_N + 80] = (int)sqrt(sqrt(toku[i * DATA_N + 0]) * toku[i * DATA_N + 43]);
			toku[i * DATA_N + 81] = (int)sqrt(toku[i * DATA_N + 43] * toku[i * DATA_N + 65]);
			toku[i * DATA_N + 82] = (int)sqrt(toku[i * DATA_N + 65] * sqrt(toku[i * DATA_N + 0]));
			toku[i * DATA_N + 83] = (int)sqrt(toku[i * DATA_N + 43] * toku[i * DATA_N + 63]);
			toku[i * DATA_N + 84] = (int)sqrt(toku[i * DATA_N + 63] * sqrt(toku[i * DATA_N + 0]));

			toku[i * DATA_N + 85] = (int)sqrt(sqrt(toku[i * DATA_N + 0]) * (10000 - toku[i * DATA_N + 43]));
			toku[i * DATA_N + 86] = (int)sqrt((10000 - toku[i * DATA_N + 43]) * toku[i * DATA_N + 65]);

			if (key == 0)
			{
				if (value1 == -1)//‘å‚«‚¢‚Ù‚Ç—Ç‚¢
					toku[i * DATA_N + DATA_N - 2] = (int)(sqrt((double)toku[i * DATA_N + key]));
				else if (value1 == 0)//¬‚³‚¢‚Ù‚Ç—Ç‚¢
					toku[i * DATA_N + DATA_N - 2] = 65535 - ((int)sqrt((double)(abs(toku[i * DATA_N + key]))));
				else
					toku[i * DATA_N + DATA_N - 2] = 65535 - ((int)sqrt((double)(abs(toku[i * DATA_N + key] - sqrt((double)value1)))));
			}
			else
			{
				if (value1 == -1)//‘å‚«‚¢‚Ù‚Ç—Ç‚¢
					toku[i * DATA_N + DATA_N - 2] = abs(toku[i * DATA_N + key]);
				else if (value1 == 0)//¬‚³‚¢‚Ù‚Ç—Ç‚¢
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

		//ƒfƒBƒŒƒNƒgƒŠ‚Ì‹æØ‚è‚ð’T‚·
		len = (int)strlen(full_path);
		for (i = len - 1; i >= 0; i--)
			if (full_path[i] == '/') break;

		p = i;//‹æØ‚èˆÊ’u‚Íp

		//file_dir‚ÉŠÖ‚µ‚Ä‚Íp”Ô–Ú‚Ü‚Å‚ðƒRƒs[‚µ‚ÄAÅŒã‚ðI’[B
		//file_name‚ÉŠÖ‚µ‚Ä‚Íp”Ô–ÚˆÈ~‚ðƒRƒs[‚µ‚ÄAÅŒã‚ðI’[B

		for (i = 0; i < p; i++)
			file_dir[i] = full_path[i];
		file_dir[i] = 0;//‚±‚±‚Ífor•¶‚ÌŠOAŽŸ‚Ì—v‘f‚ðƒ[ƒ‚É‚·‚éBiI’[)
		if (strlen(file_dir) < 1)//ƒtƒ@ƒCƒ‹–¼‚ªƒJƒŒƒ“ƒgƒpƒX‚Ìê‡
		{
			//_getcwd(file_dir, 512);//windows 7
			getcwd(file_dir, sizeof(file_dir));//linux 7
		}

		for (i = p + 1; i < len; i++)
			file_name[i - p - 1] = full_path[i];
		file_name[i - p - 1] = 0;//‚±‚±‚Ífor•¶‚ÌŠOAŽŸ‚Ì—v‘f‚ðƒ[ƒ‚É‚·‚éB(I’[)

		return(0);
	}
	int img_choose_white(unsigned char* buf1, int min, int width, int height)
	{
		//ŒÇ—§‚µ‚½•“_‚ð–„‚ß‚é
		unsigned long* label;
		int label1_n, i, j, pitch;
		int* count;

		pitch = CALC_PITCH(1, width);

		label = (unsigned long*)calloc(sizeof(unsigned long), pitch * height);//ƒ‰ƒxƒŠƒ“ƒO—Ìˆæ‚ðŠm•Û‚·‚é

		Src_label2L(label, buf1, width, height, &label1_n);//buf0‚ðƒ‰ƒxƒŠƒ“ƒO‚·‚éB

		count = (int*)calloc(sizeof(int), label1_n + 10);//ƒJƒEƒ“ƒg—Ìˆæ‚ðŠm•Û‚·‚éB

		for (j = 0; j < height; j++)//‰æ‘f”‚ðƒJƒEƒ“ƒg‚·‚é
			for (i = 0; i < width; i++)
				if (label[j * pitch + i] != 0)
					count[label[j * pitch + i]]++;

		for (j = 0; j < height; j++)
			for (i = 0; i < width; i++)
				if (label[j * pitch + i] != 0)
					if (count[label[j * pitch + i]] > min) buf1[j * pitch + i] = 0;//“Á’è‰æ‘f‚æ‚è‘å‚«‚¢—Ìˆæ‚ðÁ‚·

		free(label);
		free(count);

		return (0);
	}
	int Src_label3L(unsigned long* label1, unsigned char* buf74, int width, int height, int* label1_n)
	{
		//ƒ‰ƒxƒŠƒ“ƒOˆ—iŠÖ”•”‚ÌƒTƒuŠÖ”j
		int label_num;
		int l[4];
		int max, pitch;
		int i, j;
		int current;
		int x, y;
		unsigned long* work, * tl;

		pitch = CALC_PITCH(width, 1);

		//Žl‹÷‚ðœ‹Ž‚·‚é
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

		work = (unsigned long*)calloc(height * width + width, sizeof(unsigned long));//ì‹Æ—pƒoƒbƒtƒ@
		tl = (unsigned long*)calloc(height * width + width / 4, sizeof(unsigned long));//ì‹Æ—pƒoƒbƒtƒ@Œ´•¶‚Í1/4

		for (i = 0; i < height * width; i++) work[i] = 0;
		for (i = 0; i < height * width; i++) tl[i] = 0;

		current = 0;//Œ»Ý‚Ìƒ‰ƒxƒ‹”

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

		// ÅI“I‚ÉAƒ‰ƒxƒ‹’l‚ÌƒZƒbƒg
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
		//ƒ‰ƒxƒŠƒ“ƒOˆ—iŠÖ”•”‚ÌƒTƒuŠÖ”j
		int label_num;
		int l[4];
		int max;
		int i, j;
		int current;
		int x, y, pitch;
		unsigned long* work, * tl;

		pitch = CALC_PITCH(1, width);

		work = (unsigned long*)calloc(sizeof(unsigned long), height * pitch + pitch);//ì‹Æ—pƒoƒbƒtƒ@
		tl = (unsigned long*)calloc(sizeof(unsigned long), (height * pitch + pitch) / 4);//ì‹Æ—pƒoƒbƒtƒ@Œ´•¶‚Í1/4

		for (i = 0; i < height * pitch; i++) work[i] = 0;
		for (i = 0; i < height * pitch / 4; i++) tl[i] = 0;

		current = 0;//Œ»Ý‚Ìƒ‰ƒxƒ‹”

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

		// ÅI“I‚ÉAƒ‰ƒxƒ‹’l‚ÌƒZƒbƒg
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
	int img_max(unsigned char* buf2, unsigned char* buf1, int width, int height, int full_size0, int channel, int depth)
	{
		int pitch, i, j, k, half_size, ii, jj, full_size;
		int ss0;
		int max0[3], a[3], b[3], c[3];
		static unsigned char* buf3;
		static int initial_flag = 1;
		static int old_pitch = 0;
		static int old_height = 0;


		//“ü—Í‚ª‚QƒoƒCƒg‚Ìê‡‚ÌƒfƒoƒbƒO‚Í‚µ‚Äw‚È‚¢‚Ì‚Å’ˆÓ
		//8bit,1ch/3ch‚Í‘åä•v

		pitch = CALC_PITCH(width * channel, depth);

		if ((initial_flag == 1) || (pitch * height > old_pitch * height))
		{
			if (initial_flag != 1)
			{
				free(buf3);
			}
			buf3 = (unsigned char*)calloc(pitch * height, 1);
			old_pitch = pitch;
			old_height = height;
			initial_flag = 0;
		}

		//•½‹Ï‚ð‹‚ß‚éƒvƒƒOƒ‰ƒ€

		if (full_size0 < 3)//3x3–¢–ž‚È‚ç
		{
			pitch = CALC_PITCH(width, channel * depth);
			//MEMCPY(buf2, pitch * height, buf1, pitch * height);
			memcpy(buf2, buf1, pitch * height);
			return(0);
		}

		full_size = (full_size0 / 2) * 2 + 1;
		half_size = full_size / 2;
		pitch = CALC_PITCH(width, channel * depth);

		if (depth == 1)
		{
			for (j = 0; j < height; j++)
			{
				for (i = 0; i < width; i++)
					for (k = 0; k < channel; k++)
					{
						if (i == 0)
						{
							max0[k] = 0;//Å‰‚Ì’l

							for (ii = 0; ii <= half_size; ii++)
								if (buf1[j * pitch + ii * channel + k] > max0[k])//’´‚¦‚é’l
									max0[k] = buf1[j * pitch + ii * channel + k];
						}
						else
						{
							ii = i - half_size - 1;//Á‚·ƒf[ƒ^
							if (ii < 0) ii = 0;
							a[k] = buf1[j * pitch + ii * channel + k];

							ii = i + half_size;//‘‚â‚·ƒf[ƒ^
							if (ii > width - 1) ii = width - 1;
							b[k] = buf1[j * pitch + ii * channel + k];

							if (b[k] > max0[k]) max0[k] = b[k];//‘å‚«‚¢”Žš‚ª“ü‚Á‚Ä‚«‚½ê‡
							else if (a[k] == max0[k])//Å‘å’l‚ðÁ‚µ‚Ä‚µ‚Ü‚¤ê‡iÅ‘å’lŒvŽZ)
							{
								max0[k] = 0;
								for (ii = i - half_size; ii <= i + half_size; ii++)
								{
									if (ii < 0)        continue;
									else if (ii > width - 1)  continue;
									else
										c[k] = buf1[j * pitch + ii * channel + k];

									if (c[k] > max0[k]) max0[k] = c[k];

								}
							}
						}
						buf3[j * pitch + i * channel + k] = max0[k];
					}
			}

			for (i = 0; i < width; i++)
			{
				for (j = 0; j < height; j++)
					for (k = 0; k < channel; k++)
					{
						;
						if (j == 0)
						{
							max0[k] = 0;//Å‰‚Ì’l

							for (jj = 0; jj <= half_size; jj++)
								if (buf3[jj * pitch + i * channel + k] > max0[k])
									max0[k] = buf3[jj * pitch + i * channel + k];
						}
						else
						{
							jj = j - half_size - 1;//ˆø‚«ŽZ‚Ì•”
							if (jj < 0) jj = 0;
							a[k] = buf3[jj * pitch + i * channel + k];

							jj = j + half_size;//‰ÁŽZ‚Ì•”
							if (jj > height - 1) jj = height - 1;
							b[k] = buf3[jj * pitch + i * channel + k];

							if (b[k] > max0[k]) max0[k] = b[k];//‘å‚«‚¢”Žš‚ª“ü‚Á‚Ä‚«‚½ê‡
							else if (a[k] == max0[k])//Å‘å’l‚ðÁ‚µ‚Ä‚µ‚Ü‚¤ê‡iÅ‘å’lŒvŽZ)
							{
								max0[k] = 0;
								for (jj = j - half_size; jj <= j + half_size; jj++)
								{
									if (jj < 0)        continue;
									else if (jj > height - 1) continue;
									else
										c[k] = buf3[jj * pitch + i * channel + k];

									if (c[k] > max0[k]) max0[k] = c[k];
								}
							}
						}
						buf2[j * pitch + i * channel + k] = max0[k];
					}
			}
		}

		if (depth == 2)
		{
			PL();
			k = 0;
			for (j = 0; j < height; j++)
			{
				ss0 = 0;
				for (i = 0; i < width; i++)
				{
					if (i == 0)
					{
						max0[0] = 0;//Å‰‚Ì’l

						for (ii = 0; ii <= half_size; ii++)
							if ((*(unsigned short*)& buf1[j * pitch + ii * 2]) > max0[0])
								max0[0] = (*(unsigned short*)& buf1[j * pitch + ii * 2]);
					}
					else
					{
						ii = i - half_size - 1;//ˆø‚«ŽZ‚Ì•”
						if (ii < 0) ii = 0;
						a[0] = (*(unsigned short*)& buf1[j * pitch + ii * 2]);

						ii = i + half_size;//‰ÁŽZ‚Ì•”
						if (ii > width - 1) ii = width - 1;
						b[0] = (*(unsigned short*)& buf1[j * pitch + ii * 2]);

						if (b[0] > max0[0]) max0[0] = b[0];//‘å‚«‚¢”Žš‚ª“ü‚Á‚Ä‚«‚½ê‡
						else if (a[0] == max0[0])//Å‘å’l‚ðÁ‚µ‚Ä‚µ‚Ü‚¤ê‡iÅ‘å’lŒvŽZ)
						{
							max0[k] = 0;
							for (ii = i - half_size; ii <= i + half_size; ii++)
							{
								if (ii < 0)        continue;
								else if (ii > width - 1)  continue;
								else
									c[0] = (*(unsigned short*)& buf1[j * pitch + ii * 2]);

								if (c[0] > max0[0]) max0[k] = c[0];
							}
						}
					}
					(*(unsigned short*)& buf2[j * pitch + i * 2]) = max0[0];
				}
			}
			PL();
			for (i = 0; i < width; i++)
			{
				for (j = 0; j < height; j++)
				{
					if (j == 0)
					{
						max0[k] = 0;//Å‰‚Ì’l
						for (jj = 0; jj <= half_size; jj++)
							if ((*(unsigned short*)& buf2[jj * pitch + i * 2]) > max0[0])
								max0[0] = (*(unsigned short*)& buf2[jj * pitch + i * 2]);
					}
					else
					{
						//printf ("j=%d\n",j) ;
						jj = j - half_size - 1;//ˆø‚«ŽZ‚Ì•”
						if (jj < 0) jj = 0;
						a[0] = (*(unsigned short*)& buf2[jj * pitch + i * 2]);

						jj = j + half_size;//‰ÁŽZ‚Ì•”
						if (jj > height - 1) jj = height - 1;
						b[0] = (*(unsigned short*)& buf2[jj * pitch + i * 2]);

						if (b[0] > max0[k]) max0[0] = b[0];//‘å‚«‚¢”Žš‚ª“ü‚Á‚Ä‚«‚½ê‡
						else if (a[0] == max0[0])//Å‘å’l‚ðÁ‚µ‚Ä‚µ‚Ü‚¤ê‡iÅ‘å’lŒvŽZ)
						{
							max0[0] = 0;
							for (jj = j - half_size; jj <= j + half_size; jj++)
							{
								if (jj < 0)        continue;
								else if (jj > height - 1) continue;
								else
									c[0] = (*(unsigned short*)& buf2[jj * pitch + i * 2]);

								if (c[0] > max0[0]) max0[0] = c[0];
							}
						}
					}
					(*(unsigned short*)& buf1[j * pitch + i * 2]) = max0[0];
				}
			}
			PL();
		}

		return (0);
	}
	int img_min(unsigned char* buf2, unsigned char* buf1, int width, int height, int full_size0, int channel, int depth)
	{
		int pitch, i, j, k, half_size, ii, jj, full_size;
		int min0[3], a[3], b[3], c[3];
		static unsigned char* buf3;
		static int initial_flag = 1;
		static int old_height = 0;
		static int old_pitch = 0;

		//•½‹Ï‚ð‹‚ß‚éƒvƒƒOƒ‰ƒ€

		pitch = CALC_PITCH(width * channel, depth);

		if ((initial_flag == 1) || (pitch * height > old_pitch * height))
		{
			if (initial_flag != 1)
			{
				free(buf3);
			}
			buf3 = (unsigned char*)calloc(pitch * height, 1);
			old_pitch = pitch;
			old_height = height;
			initial_flag = 0;
		}
		//•½‹Ï‚ð‹‚ß‚éƒvƒƒOƒ‰ƒ€

		if (full_size0 < 3)//3x3–¢–ž‚È‚ç
		{
			pitch = CALC_PITCH(width, channel * depth);
			//MEMCPY(buf2, pitch * height, buf1, pitch * height);
			memcpy(buf2, buf1, pitch * height);
			return(0);
		}

		full_size = (full_size0 / 2) * 2 + 1;
		half_size = full_size / 2;
		pitch = CALC_PITCH(width, channel * depth);

		if (depth == 1)
		{
			for (j = 0; j < height; j++)
			{
				for (i = 0; i < width; i++)
					for (k = 0; k < channel; k++)
					{
						if (i == 0)
						{
							min0[k] = 255;//Å‰‚Ì’l

							for (ii = 0; ii <= half_size; ii++)
								if (buf1[j * pitch + ii * channel + k] < min0[k])//’´‚¦‚é’l
									min0[k] = buf1[j * pitch + ii * channel + k];
						}
						else
						{
							ii = i - half_size - 1;//Á‚·ƒf[ƒ^
							if (ii < 0) ii = 0;
							a[k] = buf1[j * pitch + ii * channel + k];

							ii = i + half_size;//‘‚â‚·ƒf[ƒ^
							if (ii > width - 1) ii = width - 1;
							b[k] = buf1[j * pitch + ii * channel + k];

							if (b[k] < min0[k]) min0[k] = b[k];//‘å‚«‚¢”Žš‚ª“ü‚Á‚Ä‚«‚½ê‡
							else if (a[k] == min0[k])//Å‘å’l‚ðÁ‚µ‚Ä‚µ‚Ü‚¤ê‡iÅ‘å’lŒvŽZ)
							{
								min0[k] = 255;
								for (ii = i - half_size; ii <= i + half_size; ii++)
								{
									if (ii < 0)        continue;
									else if (ii > width - 1)  continue;
									else
										c[k] = buf1[j * pitch + ii * channel + k];

									if (c[k] < min0[k]) min0[k] = c[k];

								}
							}
						}
						buf3[j * pitch + i * channel + k] = min0[k];
					}
			}

			for (i = 0; i < width; i++)
			{
				for (j = 0; j < height; j++)
					for (k = 0; k < channel; k++)
					{
						;
						if (j == 0)
						{
							min0[k] = 255;//Å‰‚Ì’l

							for (jj = 0; jj <= half_size; jj++)
								if (buf3[jj * pitch + i * channel + k] < min0[k])
									min0[k] = buf3[jj * pitch + i * channel + k];
						}
						else
						{
							jj = j - half_size - 1;//ˆø‚«ŽZ‚Ì•”
							if (jj < 0) jj = 0;
							a[k] = buf3[jj * pitch + i * channel + k];

							jj = j + half_size;//‰ÁŽZ‚Ì•”
							if (jj > height - 1) jj = height - 1;
							b[k] = buf3[jj * pitch + i * channel + k];

							if (b[k] < min0[k]) min0[k] = b[k];//‘å‚«‚¢”Žš‚ª“ü‚Á‚Ä‚«‚½ê‡
							else if (a[k] == min0[k])//Å‘å’l‚ðÁ‚µ‚Ä‚µ‚Ü‚¤ê‡iÅ‘å’lŒvŽZ)
							{
								min0[k] = 255;
								for (jj = j - half_size; jj <= j + half_size; jj++)
								{
									if (jj < 0)        continue;
									else if (jj > height - 1) continue;
									else
										c[k] = buf3[jj * pitch + i * channel + k];

									if (c[k] < min0[k]) min0[k] = c[k];
								}
							}
						}
						buf2[j * pitch + i * channel + k] = min0[k];
					}
			}
		}

		if (depth == 2)
		{
			PL();
			k = 0;
			for (j = 0; j < height; j++)
			{
				for (i = 0; i < width; i++)
				{
					if (i == 0)
					{
						min0[0] = 65535;//Å‰‚Ì’l

						for (ii = 0; ii <= half_size; ii++)
							if ((*(unsigned short*)& buf1[j * pitch + ii * 2]) < min0[0])
								min0[0] = (*(unsigned short*)& buf1[j * pitch + ii * 2]);
					}
					else
					{
						ii = i - half_size - 1;//ˆø‚«ŽZ‚Ì•”
						if (ii < 0) ii = 0;
						a[0] = (*(unsigned short*)& buf1[j * pitch + ii * 2]);

						ii = i + half_size;//‰ÁŽZ‚Ì•”
						if (ii > width - 1) ii = width - 1;
						b[0] = (*(unsigned short*)& buf1[j * pitch + ii * 2]);

						if (b[0] < min0[0]) min0[0] = b[0];//‘å‚«‚¢”Žš‚ª“ü‚Á‚Ä‚«‚½ê‡
						else if (a[0] == min0[0])//Å‘å’l‚ðÁ‚µ‚Ä‚µ‚Ü‚¤ê‡iÅ‘å’lŒvŽZ)
						{
							min0[k] = 65535;
							for (ii = i - half_size; ii <= i + half_size; ii++)
							{
								if (ii < 0)        continue;
								else if (ii > width - 1)  continue;
								else
									c[0] = (*(unsigned short*)& buf1[j * pitch + ii * 2]);

								if (c[0] < min0[0]) min0[k] = c[0];
							}
						}
					}
					(*(unsigned short*)& buf2[j * pitch + i * 2]) = min0[0];
				}
			}
			PL();
			for (i = 0; i < width; i++)
			{
				for (j = 0; j < height; j++)
				{
					if (j == 0)
					{
						min0[k] = 65535;//Å‰‚Ì’l
						for (jj = 0; jj <= half_size; jj++)
							if ((*(unsigned short*)& buf2[jj * pitch + i * 2]) < min0[0])
								min0[0] = (*(unsigned short*)& buf2[jj * pitch + i * 2]);
					}
					else
					{
						//printf ("j=%d\n",j) ;
						jj = j - half_size - 1;//ˆø‚«ŽZ‚Ì•”
						if (jj < 0) jj = 0;
						a[0] = (*(unsigned short*)& buf2[jj * pitch + i * 2]);

						jj = j + half_size;//‰ÁŽZ‚Ì•”
						if (jj > height - 1) jj = height - 1;
						b[0] = (*(unsigned short*)& buf2[jj * pitch + i * 2]);

						if (b[0] < min0[k]) min0[0] = b[0];//‘å‚«‚¢”Žš‚ª“ü‚Á‚Ä‚«‚½ê‡
						else if (a[0] == min0[0])//Å‘å’l‚ðÁ‚µ‚Ä‚µ‚Ü‚¤ê‡iÅ‘å’lŒvŽZ)
						{
							min0[0] = 65535;
							for (jj = j - half_size; jj <= j + half_size; jj++)
							{
								if (jj < 0)        continue;
								else if (jj > height - 1) continue;
								else
									c[0] = (*(unsigned short*)& buf2[jj * pitch + i * 2]);

								if (c[0] < min0[0]) min0[0] = c[0];
							}
						}
					}
					(*(unsigned short*)& buf1[j * pitch + i * 2]) = min0[0];
				}
			}
			PL();
		}

		return (0);
	}
	int img_ave(unsigned char* buf2, unsigned char* buf1, int width, int height, int full_size0, int channel, int depth)
	{
		int pitch, i, j, k, half_size, ii, jj, v, full_size;
		int ss0, ss[3];
		static unsigned char* buf3;
		static int initial_flag = 1;
		static int old_height = 0;
		static int old_pitch = 0;

		//•½‹Ï‚ð‹‚ß‚éƒvƒƒOƒ‰ƒ€

		full_size = (full_size0 / 2) * 2 + 1;

		half_size = full_size / 2;
		pitch = CALC_PITCH(width, channel * depth);

		if ((initial_flag == 1) || (pitch * height > old_pitch * height))
		{
			if (initial_flag != 1)
			{
				free(buf3);
			}
			buf3 = (unsigned char*)calloc(pitch * height, 1);
			old_pitch = pitch;
			old_height = height;
			initial_flag = 0;
		}

		if (depth == 1)
		{
			for (j = 0; j < height; j++)
			{
				ss[0] = ss[1] = ss[2] = 0;
				for (i = 0; i < width; i++)
					for (k = 0; k < channel; k++)
					{
						if (i == 0)
						{
							ss[k] = buf1[j * pitch + i * channel + k] * (half_size);
							for (ii = 0; ii <= half_size; ii++)
								ss[k] += buf1[j * pitch + ii * channel + k];
						}
						else
						{
							ii = i - half_size - 1;//ˆø‚«ŽZ‚Ì•”
							if (ii < 0) ii = 0;
							ss[k] -= buf1[j * pitch + ii * channel + k];

							ii = i + half_size;//‰ÁŽZ‚Ì•”
							if (ii > width - 1) ii = width - 1;
							ss[k] += buf1[j * pitch + ii * channel + k];
						}
						v = ss[k] / full_size;
						buf3[j * pitch + i * channel + k] = v;
					}
			}

			for (i = 0; i < width; i++)
			{
				ss[0] = ss[1] = ss[2] = 0;
				for (j = 0; j < height; j++)
					for (k = 0; k < channel; k++)
					{
						if (j == 0)
						{
							ss[k] = buf3[j * pitch + i * channel + k] * (half_size);
							for (jj = 0; jj <= half_size; jj++)
								ss[k] += buf3[jj * pitch + i * channel + k];
						}
						else
						{
							jj = j - half_size - 1;//ˆø‚«ŽZ‚Ì•”
							if (jj < 0) jj = 0;
							ss[k] -= buf3[jj * pitch + i * channel + k];

							jj = j + half_size;//‰ÁŽZ‚Ì•”
							if (jj > height - 1) jj = height - 1;
							ss[k] += buf3[jj * pitch + i * channel + k];
						}
						v = ss[k] / full_size;
						buf2[j * pitch + i * channel + k] = v;
					}
			}
		}

		if (depth == 2)
		{
			PL();
			for (j = 0; j < height; j++)
			{
				ss0 = 0;
				for (i = 0; i < width; i++)
				{
					if (i == 0)
					{
						ss0 = (*(unsigned short*)& buf1[j * pitch + i * 2]) * (half_size);
						for (ii = 0; ii <= half_size; ii++)
							ss0 += (*(unsigned short*)& buf1[j * pitch + ii * 2]);
					}
					else
					{
						ii = i - half_size - 1;//ˆø‚«ŽZ‚Ì•”
						if (ii < 0) ii = 0;
						ss0 -= (*(unsigned short*)& buf1[j * pitch + ii * 2]);

						ii = i + half_size;//‰ÁŽZ‚Ì•”
						if (ii > width - 1) ii = width - 1;
						ss0 += (*(unsigned short*)& buf1[j * pitch + ii * 2]);
					}
					v = ss0 / full_size;
					(*(unsigned short*)& buf3[j * pitch + i * 2]) = v;
				}
			}
			PL();
			for (i = 0; i < width; i++)
			{
				ss0 = 0;
				for (j = 0; j < height; j++)
				{
					if (j == 0)
					{
						ss0 = (*(unsigned short*)& buf3[j * pitch + i * 2]) * (half_size);
						for (jj = 0; jj <= half_size; jj++)
							ss0 += (*(unsigned short*)& buf3[jj * pitch + i * 2]);
					}
					else
					{
						jj = j - half_size - 1;//ˆø‚«ŽZ‚Ì•”
						if (jj < 0) jj = 0;
						ss0 -= (*(unsigned short*)& buf3[jj * pitch + i * 2]);

						jj = j + half_size;//‰ÁŽZ‚Ì•”
						if (jj > height - 1) jj = height - 1;
						ss0 += (*(unsigned short*)& buf3[jj * pitch + i * 2]);
					}
					v = ss0 / full_size;
					(*(unsigned short*)& buf1[j * pitch + i * 2]) = v;
				}
			}
			PL();
		}



		return (0);
	}
	int read_src_paramB(char* parafile, int* param, int* posi_nega, int* rate, int* step, int* evkey, int* evflag, int* keys, int* mins, int* maxs, int* para_n) //ƒpƒ‰ƒ[ƒ^“Ç‚Ýž‚Ý
	{
		int i, n;
		char linebuf[1024];
		FILE* fp;
		double para[50];

		//FOPEN(fp,parafile, "r");//windows 8
		fp = FOPEN(parafile, "r");//linux 8

		for (i = 0; i < 100; i++)
		{
			if (fgets(linebuf, sizeof(linebuf), fp) == NULL) break;
			SSCANF(linebuf, "%lf", &para[i]);
			if ((i >= 14) && ((int)para[i - 2] <= 0) && ((int)para[i - 1] <= 0) && ((int)para[i] <= 0)) break;
			if ((i >= 14) && ((int)para[i - 2] >= DATA_N) && ((int)para[i - 1] >= DATA_N) && ((int)para[i] >= DATA_N)) break;
		}
		printf("n=%d\n", i);
		n = i;//ŒÂ”

		fclose(fp);

		//			for (i=0;i<n;i++)
		//			printf ("%lf ",para[i]) ;
		//			printf ("\n");

		//“Ç‚Ý‘Ö‚¦
		param[0] = (int)para[0];
		param[1] = (int)para[1];
		param[2] = (int)para[2];
		param[3] = (int)para[3];
		param[4] = (int)para[4];

		*posi_nega = (int)para[5];
		*rate = (int)para[6];
		*step = (int)para[7];
		*evkey = (int)para[8];
		*evflag = (int)para[9];

		*para_n = (n - 10) / 3;
		for (i = 0; i < *para_n; i++)
		{
			keys[i] = (int)para[10 + i * 3];
			mins[i] = (int)para[10 + i * 3 + 1];
			maxs[i] = (int)para[10 + i * 3 + 2];
		}

		printf("posi_nega=%d,rate=%d,step=%d,evkey=%d,evflag=%d\n",
			*posi_nega, *rate, *step, *evkey, *evflag);

		for (i = 0; i < *para_n; i++)
			printf("%d: %d %d %d \n", i, keys[i], mins[i], maxs[i]);

		return(0);
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
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 0 * fsize, y0 - (8 * fsize - 1), width, height, 13, r, g, b);//1‚ÌˆÊ
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 1 * fsize, y0 - (8 * fsize - 1), width, height, 12, r, g, b);//10‚ÌˆÊ
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 2 * fsize, y0 - (8 * fsize - 1), width, height, 11, r, g, b);//10‚ÌˆÊ
		}

		else if (num == 22222)
		{
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 0 * fsize, y0 - (8 * fsize - 1), width, height, 14, r, g, b);//1‚ÌˆÊ
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 1 * fsize, y0 - (8 * fsize - 1), width, height, 11, r, g, b);//10‚ÌˆÊ
			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 2 * fsize, y0 - (8 * fsize - 1), width, height, 13, r, g, b);//10‚ÌˆÊ
		}

		else
		{
			if (num > 9999) num = num % 10000;
			if (num < 0)   num = 0;

			img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 0 * fsize, y0 - (8 * fsize - 1), width, height, num % 10, r, g, b);//1‚ÌˆÊ
			if (num >= 10)   img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 1 * fsize, y0 - (8 * fsize - 1), width, height, (num / 10) % 10, r, g, b);//10‚ÌˆÊ
			if (num >= 100)  img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 2 * fsize, y0 - (8 * fsize - 1), width, height, (num / 100) % 10, r, g, b);//10‚ÌˆÊ
			if (num >= 1000) img_write_num_9_color(buf, fsize, x0 - (5 * fsize - 1) - 5 * 3 * fsize, y0 - (8 * fsize - 1), width, height, (num / 1000) % 10, r, g, b);//10‚ÌˆÊ

			//fsize‚ÍƒtƒHƒ“ƒgƒTƒCƒY‚ÅA‚»‚Ì•ªA100ˆÊ,10ˆÊ,1ˆÊ‚Å5‚Ã‚Â‚¸‚êA
		}

		return(0);
	}
	int img_write_num_coner4_999_color(unsigned char* buf, int fsize, int width, int height, int num, int r, int g, int b)
	{
		int img_write_num_9_color(unsigned char* buf, int fsize, int x0, int y0, int width, int height, int num, int r, int g, int b);

		img_write_num_9_color(buf, fsize, width - 1 - (5 * fsize - 1) - 5 * 0 * fsize, height - 1 - (8 * fsize - 1), width, height, num % 10, r, g, b);//1‚ÌˆÊ
		img_write_num_9_color(buf, fsize, width - 1 - (5 * fsize - 1) - 5 * 1 * fsize, height - 1 - (8 * fsize - 1), width, height, (num / 10) % 10, r, g, b);//10‚ÌˆÊ
		img_write_num_9_color(buf, fsize, width - 1 - (5 * fsize - 1) - 5 * 2 * fsize, height - 1 - (8 * fsize - 1), width, height, (num / 100) % 10, r, g, b);//10‚ÌˆÊ

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
////end		//windows 9
}			//linux 9