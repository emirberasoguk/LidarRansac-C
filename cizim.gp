set terminal pngcairo size 1024,768 enhanced font 'Verdana,10'
set output 'proje_ciktisi.png'
set title 'LIDAR Veri Analizi - RANSAC ile Dogru Tespiti'
set xlabel 'X (metre)'
set ylabel 'Y (metre)'
set grid
set key outside right top
set object 1 circle at 0,0 size 0.05 fillcolor rgb 'red' fs empty border rgb 'black' lw 2 front
set arrow 1 from 0,0 to 0.855241,2.227272 dashtype 2 linecolor rgb 'red' lw 3 head filled size 0.1,15,45 front
set object 2 rectangle at 0.213810,0.556818 size 0.4,0.2 fillcolor rgb 'yellow' fs solid 1.0 border rgb 'red' front
set label 1 '2.39 m' at 0.213810,0.556818 textcolor rgb 'red' font ',12,bold' center front
set object 3 rectangle at 0.855241,2.527272 size 0.7,0.3 fillcolor rgb 'yellow' fs solid 1.0 border rgb 'black' front
set label 2 '62.2 derece (d1 & d4)' at 0.855241,2.527272 textcolor rgb 'black' font ',12,bold' center front
set label ' d1 ' at 1.245211,1.118600 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d2 ' at 1.603159,0.096463 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d3 ' at 1.605930,-2.388465 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d4 ' at 0.052992,2.350937 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d5 ' at -1.936055,-1.951595 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d6 ' at -2.950203,0.352118 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d7 ' at -2.889311,0.323481 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d8 ' at -1.311200,1.958885 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d9 ' at -1.205895,2.191712 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d10 ' at 1.674327,-2.076639 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d11 ' at -2.634494,0.867310 textcolor rgb 'black' font ',9,bold' center front boxed
set label ' d12 ' at -2.943987,0.273460 textcolor rgb 'black' font ',9,bold' center front boxed
plot 'noktalar.dat' with points pointtype 7 pointsize 0.5 linecolor rgb '#c0c0c0' title 'Tum Noktalar', \
     NaN with points pointtype 7 pointsize 1.5 linecolor rgb 'red' title 'Robot', \
     'dogru_inliers_0.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd1 noktaları (47 nokta)', \
     'dogru_segment_0.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd1', \
     'dogru_inliers_1.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd2 noktaları (8 nokta)', \
     'dogru_segment_1.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd2', \
     'dogru_inliers_2.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd3 noktaları (18 nokta)', \
     'dogru_segment_2.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd3', \
     'dogru_inliers_3.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd4 noktaları (53 nokta)', \
     'dogru_segment_3.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd4', \
     'dogru_inliers_4.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd5 noktaları (40 nokta)', \
     'dogru_segment_4.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd5', \
     'dogru_inliers_5.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd6 noktaları (8 nokta)', \
     'dogru_segment_5.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd6', \
     'dogru_inliers_6.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd7 noktaları (8 nokta)', \
     'dogru_segment_6.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd7', \
     'dogru_inliers_7.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd8 noktaları (10 nokta)', \
     'dogru_segment_7.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd8', \
     'dogru_inliers_8.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd9 noktaları (8 nokta)', \
     'dogru_segment_8.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd9', \
     'dogru_inliers_9.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd10 noktaları (32 nokta)', \
     'dogru_segment_9.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd10', \
     'dogru_inliers_10.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd11 noktaları (20 nokta)', \
     'dogru_segment_10.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd11', \
     'dogru_inliers_11.dat' with points pointtype 7 pointsize 0.7 linecolor rgb '#006400' title 'd12 noktaları (8 nokta)', \
     'dogru_segment_11.dat' with lines linewidth 5 linecolor rgb '#006400' title 'd12', \
     'hedef.dat' with points pointtype 2 pointsize 3 linewidth 3 linecolor rgb 'yellow' title '60+ Kesisim'
