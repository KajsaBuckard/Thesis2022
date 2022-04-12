
# importing libraries
import cv2
import numpy as np
  
# reading the image data from desired directory
img = cv2.imread("C:/Users/kajsa.buckard/Desktop/Thesis/Pictures/FinalTests/PicaPica/45PicaPicaDiff2.png")
cv2.imshow('Image',img)
width, height = img.size
  
# counting the number of pixels
number_of_white_pix = np.sum(img == 255)
number_of_black_pix = np.sum(img == 0)
  
print('Number of pixels:', width * height)
print('Number of white pixels:', number_of_white_pix)
print('% white pixels:', number_of_white_pix/ (width * height))
print('Number of black pixels:', number_of_black_pix)
print('% black pixels:', number_of_black_pix/ (width * height))