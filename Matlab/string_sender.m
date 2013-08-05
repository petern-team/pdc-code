% This script will ask for user input in the form of characters or
% strings and use the audio port to send this information to arduino

clear

user_input = input('Enter a string: ','s');
rate = 51200;
the_char = 0;
the_note = zeros(1,10240);

for i=1:length(user_input)
    the_char = user_input(i)+0;
    the_file = strcat('inverse_alpha/',int2str(the_char),'.wav');
    the_note = wavread(the_file);
    soundsc(the_note,rate);
end
