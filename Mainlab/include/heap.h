#ifndef LAB_HEAP_H
#define LAB_HEAP_H

#define SEGMENTSIZE 65539
#define SEGMENTCOUNT 1024

class Heap
{
public:
        Heap(int _segment_size = SEGMENTSIZE)
               { 
                    segment_size  = _segment_size;
                    current       = 0;
               }
        ~Heap()    
               {  delete_segments(); }
        void*      get_mem(int size);
        void       free_mem (void*);
private:
        struct Segment_def//���� ���������� 
        {       
                bool      used;//����������� ��� ���
                int       size;//������ ���������� �������� 
                size_t     offset;//��������, ����� ����� ���� �������
				//void* �������� �� size_t, ������ ��� ��� �������������� �������� ���� ��� ������� �������, � �������������� � ��� ���
        };

        struct Segment//�������, ��� ������� �������� ���������� ������ ������������ 
        {
                void*        data;//��������� �� ���� ������� 64��
                Segment*     prev; // ������ �� ���������� �������
                Segment_def  descriptor[SEGMENTCOUNT]; // ��������� �� ������ ������������
                int          descriptor_count; // ���-�� ������������ (������� �������� ����)
        };

        int       make_segment();//������� �� �������� ��������
        void      delete_segments();//

        int       segment_size;

        Segment*  current;
};

#endif