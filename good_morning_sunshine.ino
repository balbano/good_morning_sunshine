// Calculating the sunrise requires a 64-bit float (aka double), which the Arduino Uno does not support.
// On the uno, double is the same as float, so the sketch will still upload, but the math will not
// be accurate enough to achieve good results. For best results, use a board that supports 64-bit floats
// like the Teensy 3.1, Teensy LC, Arduino Due, etc.

void setup() {
  Serial.begin(9600);
}

void loop() {
  testJulianDate();
  testSunrise();
  testDateToTime();
  
  Serial.println("--------------------------------------");
  Serial.println();
  
  delay(5000);
}

/***************************
* JULIAN DATE CALCULATIONS *
****************************/

// Converts a Gregorian date to a Julian Day Number.
// These calculations are explained here: 
// http://www.cs.utsa.edu/~cs1063/projects/Spring2011/Project1/jdn-explanation.html
// All division should be integer division.
long gregorianToJulianDayNumber(long gregorianYear, long gregorianMonth, long gregorianDay) {
  long a = (14 - gregorianMonth) / 12;
  long y = gregorianYear + 4800 - a;
  long m = gregorianMonth + 12*a - 3;
  
  long julianDayNumber = gregorianDay + (153*m + 2)/5 + 365*y + y/4 - y/100 + y/400 - 32045;
  
  return julianDayNumber;
}

// Converts a Julian Day Number and time to a Julian Date.
// Times should be in Greenwich Mean Time (GMT).
// All division is floating point division.
double julianDayNumberToJulianDate(long julianDayNumber, long h, long m, long s) {
  double julianDate = (double)julianDayNumber + ((double)h - 12.0)/24.0 + (double)m/1440.0 + (double)s/86400.0;
  return julianDate;
}

double gregorianToJulianDate(long gregorianYear, long gregorianMonth, long gregorianDay, long h, long m, long s) {
  long julianDayNumber = gregorianToJulianDayNumber(gregorianYear, gregorianMonth, gregorianDay);
  double julianDate = julianDayNumberToJulianDate(julianDayNumber, h, m, s);
  return julianDate;
}

String julianDateToTime(double julianDate) {
  double timePortion = (julianDate - 0.5) - floor(julianDate - 0.5);
  
  double fractionalHours = timePortion * 24.0;
  double fractionalMinutes = timePortion * 1440.0;
  double fractionalSeconds = timePortion * 86400.0;
  
  int h = floor(fractionalHours);
  int m = floor(fmod(fractionalMinutes, 60.0));
  int s = floor(fmod(fractionalSeconds, 60.0));
  
  String time = String(h) + ":" + String(m) + ":" + String(s);
  
  return time;
}

/***********************
* SUNRISE CALCULATIONS *
************************/

// Calculate sunrise: http://en.wikipedia.org/wiki/Sunrise_equation#Complete_calculation_on_Earth
// J_date is the Julian Date, l_w is the longitude west.

long julianCycle(double J_date, double l_w) {
  double n_star = J_date - 2451545.0009 - l_w/360.0;
  long n = floorf(n_star + 0.5); //
  return n; // n is the Julian cycle since Jan 1st, 2000.
}

double approximateSolarNoon(double J_date, double l_w) {
  double n = julianCycle(J_date, l_w);
  double J_star = 2451545.0009 + l_w/360.0 + n;
  return J_star; // J_star is an approximation of solar noon at l_w.
}

double solarMeanAnamoly(double J_date, double l_w) {
  double J_star = approximateSolarNoon(J_date, l_w);
  double M = fmod(357.5291 + 0.98560028*(J_star - 2451545.0), 360.0);
  return M; // M is the mean anomaly.
}

double equationOfCenter(double J_date, double l_w) {
  double M = solarMeanAnamoly(J_date, l_w);
  double C = 1.9148*sin(M*M_PI/180.0) + 0.0200*sin(2*M*M_PI/180.0) + 0.0003*sin(3*M*M_PI/180.0);
  return C; // C is the equation of the center.
}

double eclipticLongitude(double J_date, double l_w) {
  double M = solarMeanAnamoly(J_date, l_w);
  double C = equationOfCenter(J_date, l_w);
  double lambda = fmod(M + 102.9372 + C + 180.0, 360.0);
  return lambda; // lambda is the ecliptic longitude.
}

double solarTransit(double J_date, double l_w) {
  double J_star = approximateSolarNoon(J_date, l_w);
  double M = solarMeanAnamoly(J_date, l_w);
  double lambda = eclipticLongitude(J_date, l_w);
  double J_transit = J_star + 0.0053*sin(M*M_PI/180.0) - 0.0069*sin(2*lambda*M_PI/180.0);
  return J_transit; // J_transit is the hour angle for solar transit (or solar noon).
}

double declinationOfTheSun(double J_date, double l_w) {
  double lambda = eclipticLongitude(J_date, l_w);
  double delta = asin(sin(lambda*M_PI/180.0) * sin(23.45*M_PI/180.0)) * 180.0/M_PI;
  return delta; // delta is the declination of the sun.
}

// Phi is the north latitude.
double hourAngle(double J_date, double l_w, double phi) {
  double delta = declinationOfTheSun(J_date, l_w);
  double omega_deg = acos((sin(-0.83*M_PI/180.0) - sin(phi*M_PI/180.0)*sin(delta*M_PI/180.0) / (cos(phi*M_PI/180.0) * cos(delta*M_PI/180.0)))) * 180.0/M_PI;
  return omega_deg; // omega_deg is the hour angle.
}

// J_date is the Julian Date, l_w is the longitude west. Phi is the north latitude.
double sunrise(double J_date, double l_w, double phi) {
  double J_transit = solarTransit(J_date, l_w);
  double omega_deg = hourAngle(J_date, l_w, phi);
  double J_rise = J_transit - omega_deg/360.0;
  return J_rise; // J_rise is the actual Julian Date of sunrise.
}

/********
* TESTS *
*********/

void testJulianDate() {
  // Julian date from http://aa.usno.navy.mil/cgi-bin/aa_jdconv.pl
  // The Julian date for CE 2015 April 17 16:44:20.0 UT is
  // JD 2457130.197454
  
  double julianDate = gregorianToJulianDate(2015, 4, 17, 16, 44, 20);
    
  Serial.println("The Julian date for 2015 April 17 16:44:20 is:   ");
  Serial.print("Expected:   ");
  Serial.println("2457130.197454");
  Serial.print("Calculated: ");
  Serial.print(julianDate);
  Serial.println(" (rounded by serial.print()).");
  Serial.print("Back to normal represenation of time: ");
  Serial.println(julianDateToTime(julianDate));
  Serial.println();
}

void testSunrise() {
  // Sunrise for Chicago: 87.6847° W, 41.8369° N
  // 2015 April 17 16:44:20.0 UT is
  // http://www.esrl.noaa.gov/gmd/grad/solcalc/sunrise.html
  // Equation of Time: 0.3, Solar Declination: 10.37deg, Solar Noon: 17:50:17 UTC
  // Sunrise: 11:08 UTC.
  
  double julianDate = gregorianToJulianDate(2015, 4, 17, 16, 44, 20);
  double J_rise = sunrise(julianDate, 87.6847, 41.8369);
  double expected_J_rise = gregorianToJulianDate(2015, 4, 17, 11, 8, 0);
  
  Serial.println("Julian Date of sunrise in Chicago on 2015 April 17 16:44:20 is: ");
  Serial.print("Expected Julian:   ");
  Serial.println(expected_J_rise);
  Serial.print("Calculated Julian: ");
  Serial.println(J_rise);
  Serial.print("Difference: ");
  Serial.println(expected_J_rise - J_rise);
  Serial.print("Difference * 10000: ");
  Serial.println(10000 * (expected_J_rise - J_rise));
  Serial.println();
  
  Serial.print("Expected UTC:   ");
  Serial.println("11:08");
  Serial.print("Expected Calculated UTC: ");
  Serial.println(julianDateToTime(expected_J_rise));
  Serial.print("Calculated UTC: ");
  Serial.println(julianDateToTime(J_rise));
  Serial.println();
}

void testDateToTime() {
  Serial.println("Converting 2015-04-24 7:8:9 to Julian Date and back again: ");
  Serial.println(julianDateToTime(gregorianToJulianDate(2015, 4, 24, 7, 8, 9)));
  Serial.println();
}
