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
c = 3e8;

w_pe = 2.8e7; % As used in the unittests
w_ce = e * Btot / me;
w_uh = sqrt(w_pe^2 + w_ce^2);
w_r = w_ce/2 + sqrt(w_pe^2 + w_ce^2/4);
w_l = -w_ce/2 + sqrt(w_pe^2 + w_ce^2/4);

% load O-waves
load ../Debug/data_IonosphereO_WaveTest.dat
frequencies_O = data_IonosphereO_WaveTest(:,4);
w_O = 2 * pi * frequencies_O;
n_sq_simulated = data_IonosphereO_WaveTest(:,2);

k_O = sqrt(n_sq_simulated.^2 .* (2*pi*w_O).^2 / c^2);

% load X-waves
load ../Debug/data_IonosphereX_WaveTest_1.dat
load ../Debug/data_IonosphereX_WaveTest_2.dat
frequencies_X_1 = data_IonosphereX_WaveTest_1(:,4);
frequencies_X_2 = data_IonosphereX_WaveTest_2(:,4);
w_X_1 = 2 * pi * frequencies_X_1;
w_X_2 = 2 * pi * frequencies_X_2;
n_sq_simulated_x_1 = data_IonosphereX_WaveTest_1(:,2);
n_sq_simulated_x_2 = data_IonosphereX_WaveTest_2(:,2);

% ignore empty values
w_X_1(w_X_1 < 1000) = nan;
w_X_2(w_X_2 < 1000) = nan;

k_X_1 = sqrt(n_sq_simulated_x_1.^2 .* (2*pi*w_X_1).^2 / c^2);
k_X_2 = sqrt(n_sq_simulated_x_2.^2 .* (2*pi*w_X_2).^2 / c^2);

% Plot test results
figure
hold on
plot(k_X_1, w_X_1, 'r*');
plot(k_X_2, w_X_2, 'ro');
plot(k_O, w_O, 'b*');
xlim([0.01 1])
ylim([1e7 5e7])

% Plot support lines
line([0 1], [w_uh w_uh], 'LineStyle', '-.', 'Color', 'k');
line([0 1], [w_pe w_pe], 'Color', 'k');
line([0 1], [w_l w_l], 'LineStyle', '-.', 'Color', 'k');
line([0 1], [w_r w_r], 'LineStyle', '-.', 'Color', 'k');



% legends, text etc
text(0.7, w_pe, '\omega_{pe}', 'VerticalAlignment','top');
text(0.7, w_r, '\omega_R', 'VerticalAlignment','bottom');
text(0.6, w_uh, '\omega_{UH}', 'VerticalAlignment','bottom');
text(0.7, w_l, '\omega_l', 'VerticalAlignment','top');
legend('X-mode n_1','X-mode n_2', 'O-mode', 'Location', 'SouthEast');

