#include "pch.h"
#include "ImageDetection.h"
using cv::Mat;

//��̬��Ա��ʼ��
AssistConfig* ImageDetection::m_AssistConfig = AssistConfig::GetInstance();


//�෽��ʵ��
ImageDetection::ImageDetection()
{
    initImg();

    //����AIģ��
    initDnn();
}

ImageDetection::~ImageDetection()
{
    //ͼ����Դ�ͷ�
    releaseImg();

    //�ͷ���Դ����
    try {
        if (m_net != NULL) {
            delete m_net;
            m_net = NULL;
        }
    }
    catch (Exception ex) {
        string msg = "";
    }
}

//�޸����ú���Ҫ���³�ʼ��һЩ����
void ImageDetection::ReInit() {
    releaseImg();
    initImg();
}

//��ʼ��ͼ����Դ
void ImageDetection::initImg(){
    //ע����Ļ���ź�����
    //ע��ץȡ��Ļ��ʱ��ʹ�����ź�������������꣬ץȡ��������ʵ�����߼��ֱ�������
    cv::Rect detectRect = m_AssistConfig->detectRect;
    cv::Rect detectZoomRect = m_AssistConfig->detectZoomRect;


    // ��ȡ��Ļ DC
    m_screenDC = GetDC(HWND_DESKTOP);
    m_memDC = CreateCompatibleDC(m_screenDC);
    // ����λͼ
    m_hBitmap = CreateCompatibleBitmap(m_screenDC, detectRect.width, detectRect.height);
    SelectObject(m_memDC, m_hBitmap);

    //����λͼ��Ϣͷ
    int iBits = GetDeviceCaps(m_memDC, BITSPIXEL) * GetDeviceCaps(m_memDC, PLANES);
    WORD wBitCount;
    if (iBits <= 1)
        wBitCount = 1;
    else if (iBits <= 4)
        wBitCount = 4;
    else if (iBits <= 8)
        wBitCount = 8;
    else if (iBits <= 24)
        wBitCount = 24;
    else
        wBitCount = 32;
    m_Bitmapinfo = new BITMAPINFO{ {sizeof(BITMAPINFOHEADER), detectRect.width, -detectRect.height, 1, wBitCount, BI_RGB },{0,0,0,0} };

    //�������ͼ�����ݵ�mat
    //m_mat.create(detectRect.height, detectRect.width, CV_8UC4);
    //m_mat3.create(detectRect.height, detectRect.width, CV_8UC3);
}

//�ͷ�ͼ����Դ
void ImageDetection::releaseImg() {

    //��Դ�ͷ�
    try {
        m_mat_src.release();
        m_mat_3.release();

        if (m_Bitmapinfo != NULL)
            delete m_Bitmapinfo;
        DeleteObject(m_hBitmap);
        DeleteDC(m_memDC);
        ReleaseDC(HWND_DESKTOP, m_screenDC);
    }
    catch (Exception ex) {
        string msg = "";
    }

    m_Bitmapinfo = NULL;
    m_hBitmap = NULL;
    m_memDC = NULL;
    m_screenDC = NULL;
}


/* ��ʼ��ģ�� */
void ImageDetection::initDnn(){

    // ����ģ���ļ�
    //opencv��dnnģ��(NVIDIA GPU������ģ��)
    m_net = new cv::dnn::DetectionModel(ModelFile, ConfigFile);

    // �������м���
    //(*m_net).setPreferableBackend(dnn::DNN_BACKEND_CUDA);
    //(*m_net).setPreferableTarget(dnn::DNN_TARGET_CUDA);
    (*m_net).setPreferableBackend(dnn::DNN_BACKEND_CUDA);
    (*m_net).setPreferableTarget(dnn::DNN_TARGET_CUDA);

    (*m_net).setInputSize(320, 320);
    //(*m_net).setInputSize(512, 512);
    (*m_net).setInputScale(1.0 / 255.0);
    (*m_net).setInputMean((127.5, 127.5, 127.5));
    //(*m_net).setInputSwapRB(true);

    // ���ط����ǩ
    ifstream fin(LabelFile);
    if (fin.is_open())
    {
        string className = "";
        while (std::getline(fin, className))
            m_classLabels.push_back(className);
    }

    return;
}


/* ��ȡ���������Ļ��ͼ */
void ImageDetection::getScreenshot()
{
    //������Ļ���ź�ģ��ü����ʵ��ͼ��������
    //ע��ץȡ��Ļ��ʱ��ʹ�����ź�������������꣬ץȡ��������ʵ�����߼��ֱ�������
    cv::Rect detectRect = m_AssistConfig->detectRect;
    cv::Rect detectZoomRect = m_AssistConfig->detectZoomRect;
   

    // �õ�λͼ������
    // ʹ��BitBlt��ͼ�����ܽϵͣ������޸�ΪDXGI
    //Windows8�Ժ�΢��������һ���µĽӿڣ��С�Desktop Duplication API����Ӧ�ó��򣬿���ͨ������API�����������ݡ�
    //����Desktop Duplication API��ͨ��Microsoft DirectX Graphics Infrastructure (DXGI)���ṩ����ͼ��ģ��ٶȷǳ��졣
    bool opt = BitBlt(m_memDC, 0, 0, detectRect.width, detectRect.height, m_screenDC, detectZoomRect.x, detectZoomRect.y, SRCCOPY);

    //ע���ڷ�opencv������ʹ��matʱ����Ҫ�ֶ�����create�����ڴ洴�����飻�ظ�ִ��create�������ᵼ���ظ��������ݴ�����飻
    m_mat_src.create(detectRect.height, detectRect.width, CV_8UC4);
    //int rows = GetDIBits(m_screenDC, m_hBitmap, 0, detectRect.height, m_mat.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    int rows = GetDIBits(m_memDC, m_hBitmap, 0, detectRect.height, m_mat_src.data, (BITMAPINFO*)m_Bitmapinfo, DIB_RGB_COLORS);

    //cv::namedWindow("opencvwindows", WINDOW_AUTOSIZE);
    //cv::imshow("opencvwindows", m_mat_src);
    //waitKey(2000);

    //��Ļ��ͼ����Ƶ��ͼ��ʽ��һ������Ҫ����ͼ���ʽת��
    //ȥ��alpha ֵ(͸����)ͨ����ת��ΪCV_8UC3��ʽ
    cv::cvtColor(m_mat_src, m_mat_3, COLOR_RGBA2RGB);

    //���ݸ�ʽת��
    //m_mat_3.convertTo(img_mat, CV_8UC3, 1.0);

    return;
}


/* AI���� */
DETECTRESULTS ImageDetection::detectImg()
{
    //������Ļ���ź�ģ��ü����ʵ��ͼ��������
    //ע��ץȡ��Ļ��ʱ��ʹ�����ź�������������꣬ץȡ��������ʵ�����߼��ֱ�������
    cv::Rect detectRect = m_AssistConfig->detectRect;
    cv::Rect detectZoomRect = m_AssistConfig->detectZoomRect;
    int gameIndex = m_AssistConfig->gameIndex;
    int playerCentX = m_AssistConfig->playerCentX;

    std::vector< int > classIds;
    std::vector< float > confidences;
    std::vector< Rect > boxes;
    DETECTRESULTS out;

    try
    {
        //ִ��ģ������
        //Mat mat = m_mat_3.clone();
        m_net->detect(m_mat_3, classIds, confidences, boxes, MinConfidence);

        //����������
        float maxConfidence = 0.0; //������Ŷ�����λ��
        for (int i = 0; i < classIds.size(); i++) {

            //��������������͡����ŶȺ�����
            int classid = classIds.at(i);
            float confidence = confidences.at(i);
            if (classid == PersonClassId && confidence > MinConfidence) {

                //�Ѹ��������ļ�����ŵ�������
                Rect box = boxes.at(i);

                //Ϊ������Ŀ���ų�̫�����̫С��ģ��
                if (box.width <= 280 && box.width >= 10 && box.height <= 320 && box.height >= 10)
                {
                    //�ж��Ƿ�����Ϸ���߱���,ģ��λ��Ϊ��Ļ��Ϸ��λ��
                    //��Ϸ�ߵ�λ������Ļ�·�����һ�㣬��� 860/1920��
                    //������Ϸ������ҡ�ڷ��Ƚϴ�����x��ļ���ֵҪ���ô�һЩ��
                    /*
                    if (gameIndex == 0 &&  //����������Ϸ����Ҫ���⴦��
                        abs(box.x + box.width / 2 - playerCentX) <= 100 &&
                        box.y > detectRect.height * 1 / 2 &&
                        abs(detectRect.height - (box.y + box.height)) <= 10)
                    {
                        //�ų���Ϸ���Լ�
                        //var testi = 0;
                    }
                    else
                    {
                        //���������⵽�Ķ���
                        out.classIds.push_back(classid);
                        out.confidences.push_back(confidence);
                        out.boxes.push_back(box);

                        //�������Ŷ�������Ա��λ��
                        if (confidence > maxConfidence) {
                            maxConfidence = confidence;
                            out.maxPersonConfidencePos = out.classIds.size() - 1;
                        }
                    }
                    */

                    //���������⵽�Ķ���
                    out.classIds.push_back(classid);
                    out.confidences.push_back(confidence);
                    out.boxes.push_back(box);

                    //�������Ŷ�������Ա��λ��
                    if (confidence > maxConfidence) {
                        maxConfidence = confidence;
                        out.maxPersonConfidencePos = out.classIds.size() - 1;
                    }
                }
            }
        }

    }
    catch (Exception ex) {
        string msg = "";
    }

    return out;
}


cv::Mat ImageDetection::getImg() {
    //��¡mat���ݽ��ⲿ����ʹ�ã����������ֻ��������mat�������ǵ������ڴ���
    Mat mat = m_mat_3.clone();
    return mat;
}