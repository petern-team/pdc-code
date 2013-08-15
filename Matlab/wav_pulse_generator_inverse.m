%write a .wav file that encodes bits of 0 and 1
%of length (sample rate)/2
clear

% Frequency of pulses is rate/pts_per_period
frequency = 6400;
pulses_per_file = 20;
pts_per_period = 8;

%Arduino will read periods_per_pulse/4 values per pulse at rate 51200
periods_per_pulse = 32;%116;%232;%464;   

% length of each pulse is pts_per_pulse/rate = periods_per_pulse/frequency
pts_per_pulse = pts_per_period*periods_per_pulse;

rate = frequency*pts_per_period; %rate the sound file will be played at. 

to_encode_array = zeros(255,8);
the_byte = zeros(1,8);
ascii = char(zeros(1,255));

%write the binary matrices for each ascii character into separate rows
%of an array
for i=1:255
    ascii(i) = char(i);
    the_byte = de2bi(i);
    for j=1:8
        if (9-j)>size(the_byte,2)
            to_encode_array(i,j) = 0;
        else
            to_encode_array(i,j) = the_byte(9-j);
        end
    end
end

square_wave = zeros(255,pulses_per_file);

% add a header to each binary square wave; this is what the arduino will
% eventually see
for i=1:255
    the_byte = to_encode_array(i,:);
    for j=1:6               % put the header at the beginning of the wave
        if j < 2 || j > 4
            square_wave(i,j) = 0;
        else
            square_wave(i,j) = 1;
        end
    end
    for j=7:14
        square_wave(i,j) = the_byte(j-6);
    end
    for j=15:pulses_per_file
        square_wave(i,j) = 0;
    end
    %square_wave(i,pulses_per_file) = 1;
    %wavwrite(wav_files(i,:),samp_per_sec,strcat(char(i),'wav'));
end

% transform the square wave into a sinewave pulse so that the soundcard
% can handle it. The points per period of oscillation, periods per "pulse"
% (binary 0 or 1), and points per period can be adjusted at the top of
% the file

sine_pulse = zeros(255,pulses_per_file*pts_per_pulse);
the_code = zeros(1,pulses_per_file);
for i=1:255
    the_code = square_wave(i,:);
    for j=1:pulses_per_file
        for k=1:pts_per_pulse
            col_index = (j-1)*pts_per_pulse+k;
            if the_code(j) ~= 0             % this is switched for inverse encoding
                sine_pulse(i,col_index) = 0;
            else
                sine_pulse(i,col_index) = ((k*periods_per_pulse)/pts_per_pulse)*2*pi;
            end
        end
    end
end

wav_files = zeros(255,pulses_per_file*pts_per_pulse);

% transform the raw points into a sinewave
for i=1:255
    for j=1:(pulses_per_file*pts_per_pulse)
        wav_files(i,j) = sin(sine_pulse(i,j));
    end
end


for i=32:126
% for 1=97:122 do this to output only the lowercase alphabet
    wavwrite(wav_files(i,:),rate,strcat('inverse_alpha/',int2str(i)));
end
% plot(wav_files(1,:))