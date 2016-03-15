% Run the ionospheric tests first:
% in the main folder, execute:
% make clean
% make test
% ./ionosphericraytracertest
clc
close all
clear all

Btot = 0.00005;
e = 1.6021765e-19;  % the unit charge [C]
me = 9.10938356e-31; % The mass of electron;

w_pe = 2.8e7; % As used in the unittests
w_ce = e * Btot / me;
w_uh = sqrt(w_pe^2 + w_ce^2);
w_r = w_ce/2 + sqrt(w_pe^2 + w_ce^2/4);
w_l = -w_ce/2 + sqrt(w_pe^2 + w_ce^2/4);

% load O-waves
load ../Debug/data_IonosphereO_WaveTest.dat
frequencies = data_IonosphereO_WaveTest(:,4);
n_sq_simulated = data_IonosphereO_WaveTest(:,2);
figure

% load X-waves
load ../Debug/data_IonosphereX_WaveTest_1.dat;
load ../Debug/data_IonosphereX_WaveTest_2.dat;
n_sq_simulated_X_1 = data_IonosphereX_WaveTest_1(:,2);
n_sq_simulated_X_2 = data_IonosphereX_WaveTest_2(:,2);
hold on

% ignore empty values
n_sq_simulated(n_sq_simulated == 0) = nan;
n_sq_simulated_X_1(n_sq_simulated_X_1 == 0) = nan;
n_sq_simulated_X_2(n_sq_simulated_X_2 == 0) = nan;

% plot the waves
plot(frequencies, n_sq_simulated_X_1, 'ro')
plot(frequencies, n_sq_simulated_X_2, 'r*')
plot(frequencies, n_sq_simulated, 'bo')

% Plot plasma frequency (single point at n^2=0)
f_pe = w_pe/(2*pi);
line([f_pe f_pe], [0 2], 'LineStyle', '-', 'Color', 'k')
line([w_uh/(2*pi) w_uh/(2*pi)], [0 2], 'LineStyle', '-.', 'Color', 'k')
line([w_l/(2*pi) w_l/(2*pi)], [0 2], 'LineStyle', '-.', 'Color', 'k')
line([w_r/(2*pi) w_r/(2*pi)], [0 2], 'LineStyle', '-.', 'Color', 'k')

% Plot line n^2 = 1
line([0 3e7], [1 1], 'LineStyle', '-', 'Color', 'k')

xlim([0 1e7])
ylim([0, 2])

% legends, text etc
text(w_l/(2*pi), 1.05, '\omega_L\rightarrow', 'HorizontalAlignment','right');
text(w_r/(2*pi), 1.05, '\leftarrow\omega_R');
text(w_uh/(2*pi), 1.7, '\leftarrow\omega_{UH}');
text(w_pe/(2*pi), 1.85, '\leftarrow\omega_p');
legend('X-mode n_1','X-mode n_2', 'O-mode');
