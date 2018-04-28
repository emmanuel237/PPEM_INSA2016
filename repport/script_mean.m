%script to extract the the number from the consol text capture
file_name = input('enter the name of the file (with the extension) : ');
fid = fopen(file_name);
measure = 0;
tline = fgetl(fid);
n = 1;
while ischar(tline)%reading the content of the file and extracting the number
 space_ptr = strfind(tline, ' ');
 if length(space_ptr) == 8
  measure(n) = str2double(tline(int8(space_ptr(7)):int8(space_ptr(8))));
   n = n + 1;
 end
 tline = fgetl(fid);
end
fprintf('Samples mean : %i\n',round(mean(measure)));



fclose(fid);