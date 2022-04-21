import java.util.Random;
import java.util.concurrent.*;
import java.io.*;

public class Main {

    public static int array_size = 100;
	
	public static int[] readArray(String filename){
		int[] array = new int[array_size];
		try{
			BufferedReader in = new BufferedReader(new FileReader(filename));
			String str;
			str = in.readLine();
			String[] strArr = str.split(" ");
			array_size = strArr.length;
			array = new int[array_size];
			int i = 0;
			for(String s : strArr){
				array[i] = Integer.parseInt(s);
				++i;
			}
		}catch (IOException e){
		}finally{
			return array;
		}
	}


    public static void main(String[] args) {
	    ForkJoinPool pool = new ForkJoinPool();
	    int []array1 = readArray("test.txt");
	    int []array2 = array1.clone();
	    System.out.println("Original Array: ");
        for (int value : array1) {
            System.out.printf("%d ", value);
        }
	    System.out.print("\n\n");

        //QuickSort
        QuickSort quicksort = new QuickSort(array1,0,array1.length-1);
        pool.invoke(quicksort);
        System.out.println("QuickSort: ");
        for (int value : array1) {
            System.out.printf("%d ", value);
        }
        System.out.print("\n");

        //MergeSort
        MergeSort mergesort = new MergeSort(array2,0,array2.length-1);
        pool.invoke(mergesort);
        System.out.println("MergeSort: ");
        for (int value : array2) {
            System.out.printf("%d ", value);
        }
        System.out.print("\n");
        

    }

}


class QuickSort extends RecursiveAction {
    static final int THRESHOLD = 5;

    private int[] array;
    private int begin;
    private int end;

    public QuickSort(int[] array, int begin, int end) {
        this.array = array;
        this.begin = begin;
        this.end = end;
    }

    @Override
    protected void compute() {
        if(end < begin) {
            return ;
        }
        if (end - begin < THRESHOLD) {
            // simple sort
            int tmp = 0;
            for (int i = begin+1; i <= end; i++)
            {
                tmp = array[i];
                int j;
                for (j=i-1; j>=begin && array[j] > tmp; --j ) {
                    array[j+1] = array[j];
                }
                array[j+1] = tmp;
            }
        }
        else {
            // quicksort
            int pivot = array[begin];
            int k = begin+1;

            for (int i = begin + 1; i<= end; ++i) {
                if(array[i] < pivot) {
                    int tmp= array[i];
                    array[i] = array[k];
                    array[k] = tmp;
                    k+=1;
                }
            }
            array[begin] = array[k-1];
            array[k-1] = pivot;

            QuickSort leftTask = new QuickSort(array, begin, k-2);
            QuickSort rightTask = new QuickSort(array, k, end);

            leftTask.fork();
            rightTask.fork();

            leftTask.join();
            rightTask.join();
        }
    }
}

class MergeSort extends RecursiveAction {
    static final int THRESHOLD = 5;

    private int[] array;
    private int begin;
    private int end;

    public MergeSort(int[] array, int begin, int end) {
        this.array = array;
        this.begin = begin;
        this.end = end;
    }

    @Override
    protected void compute() {
        if (end - begin < THRESHOLD) {
            // simple sort
            int tmp = 0;
            for (int i = begin+1; i <= end; i++)
            {
                tmp = array[i];
                int j;
                for (j=i-1; j>=begin && array[j] > tmp; --j ) {
                    array[j+1] = array[j];
                }
                array[j+1] = tmp;
            }
        }
        else {
            // mergesort
            int mid = (begin+end)/2;

            MergeSort leftTask = new MergeSort(array, begin, mid);
            MergeSort rightTask = new MergeSort(array, mid+1, end);

            leftTask.fork();
            rightTask.fork();

            leftTask.join();
            rightTask.join();

            merge(begin,mid,end);
        }
    }
	//merge
    private void merge(int left, int mid, int right) {
        int temp [] = new int[right - left + 1];
        int x = left;
        int y = mid + 1;
        int z = 0;
        while (x <= mid && y <= right) {
            if (array[x] <= array[y]) {
                temp[z++] = array[x++];
            } else {
                temp[z++] = array[y++];
            }
        }
        while (y <= right) {
            temp[z++] = array[y++];
        }
        while (x <= mid) {
            temp[z++] = array[x++];
        }

        for (z = 0; z < temp.length; z++) {
            array[left + z] = temp[z];
        }
    }
}

