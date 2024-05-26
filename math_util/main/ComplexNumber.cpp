#include "ComplexNumber.hpp"

ComplexNumber::ComplexNumber (float Input1, float Input2, int form)
    : Real (Input1), Imaginary (Input2), Magnitude (Input1), Argument (Input2) {
  if (form == CARTESIAN)
    SetPolarForm ();
  else if (form == POLAR)
    SetCartesianForm ();
  else
    std::cout << "Warning: Invalid ComplexNumber Form!" << std::endl;
}

float ComplexNumber::GetReal () const { return Real; }
float ComplexNumber::GetImaginary () const { return Imaginary; }
float ComplexNumber::GetMagnitude () const { return Magnitude; }
float ComplexNumber::GetArgument () const { return Argument; }

// if complex parameters change, recalculate complementary complex form
void ComplexNumber::SetReal (float input) {
  Real = input;
  SetPolarForm ();
}
void ComplexNumber::SetImaginary (float input) {
  Imaginary = input;
  SetPolarForm ();
}
void ComplexNumber::SetMagnitude (float input) {
  Magnitude = input;
  SetCartesianForm ();
}
void ComplexNumber::SetArgument (float input) {
  Argument = input;
  SetCartesianForm ();
}

void ComplexNumber::SetCartesianForm () {
  CalculateReal ();
  CalculateImaginary ();
}

void ComplexNumber::SetPolarForm () {
  CalculateMagnitude ();
  CalculateArgument ();
}

void ComplexNumber::CalculateReal () {
  Real = Magnitude * cos (Argument);
}

void ComplexNumber::CalculateImaginary () {
  Imaginary = Magnitude * sin (Argument);
}

void ComplexNumber::CalculateMagnitude () {
  Magnitude = sqrt (pow (Real, 2) + pow (Imaginary, 2));
}

void ComplexNumber::CalculateArgument () {
  // answer presented between -PI and +PI
  // check for 1st quadrant and 4th quadrant
  if (Real > 0 && Imaginary >= 0 || Real > 0 && Imaginary <= 0) {
    Argument = atan (Imaginary / Real);
  }
  // check for 2nd quadrant
  else if (Real < 0 && Imaginary >= 0) {
    Argument = PI + atan (Imaginary / Real);
  }
  // check for 3rd quadrant
  else if (Real < 0 && Imaginary <= 0) {
    Argument = -PI + atan (Imaginary / Real);
  }
  // check for divide by 0 instances on positive Imaginary line
  else if (Real == 0 && Imaginary > 0) {
    Argument = PI / 2;
  }
  // check for divide by 0 instances on negative Imaginary line
  else if (Real == 0 && Imaginary < 0) {
    Argument = -PI / 2;
  }
  // else it's probably 0/0 indeterminate
  else {
    Argument = 0;
  }
}

void ComplexNumber::PrintCartesianForm () const {
  std::cout << Real << std::showpos << Imaginary << std::noshowpos << "i " << std::endl;
}

void ComplexNumber::PrintPolarForm (int angleType) const {
  float arg = 0.0;
  std::string str = "";
  if (angleType == DEGREES) {
    arg = Degrees (Argument);
    str = "degrees";
  } else {
    arg = Argument;
    str = "radians";
  }

  std::cout << Magnitude << ", " << arg << " " << str << std::endl;
}

std::ostream &operator<< (std::ostream &os, const ComplexNumber &rhs) {
  os << rhs.GetReal () << std::showpos << rhs.GetImaginary () << std::noshowpos << "i";
  return os;
}

std::istream &operator>> (std::istream &is, ComplexNumber &rhs) {
  // assuming cartesian form as input from stream
  float tempReal = 0.0;
  float tempImaginary = 0.0;

  std::string s;
  std::getline (is, s);
  std::stringstream ss (s);

  ss >> tempReal;
  ss >> tempImaginary;
  rhs.SetReal (tempReal);
  rhs.SetImaginary (tempImaginary);

  return is;
}

ComplexNumber operator+ (const ComplexNumber &lhs, const ComplexNumber &rhs) {
  ComplexNumber result (lhs.GetReal () + rhs.GetReal (), lhs.GetImaginary () + rhs.GetImaginary ());
  return result;
}

ComplexNumber operator- (const ComplexNumber &lhs, const ComplexNumber &rhs) {
  ComplexNumber result (lhs.GetReal () - rhs.GetReal (), lhs.GetImaginary () - rhs.GetImaginary ());
  return result;
}

ComplexNumber operator* (const ComplexNumber &lhs, const ComplexNumber &rhs) {
  ComplexNumber result (lhs.GetMagnitude () * rhs.GetMagnitude (), lhs.GetArgument () + rhs.GetArgument (), POLAR);
  return result;
}

ComplexNumber operator/ (const ComplexNumber &lhs, const ComplexNumber &rhs) {
  ComplexNumber result (lhs.GetMagnitude () / rhs.GetMagnitude (), lhs.GetArgument () - rhs.GetArgument (), POLAR);
  return result;
}

ComplexNumber logC (const ComplexNumber &X) {
  // takes the natural lograithm of a complex number
  const float tempMag = X.GetMagnitude ();
  const float tempArg = X.GetArgument ();
  ComplexNumber result (log (tempMag), tempArg);
  return result;
}

// return result of complex number raised to the power of another complex number
ComplexNumber powC (const ComplexNumber &X, const ComplexNumber &Y) {
  // takes complex number X to the power of complex number Y
  // X = a + bi
  const float a = X.GetReal ();
  const float b = X.GetImaginary ();

  // Y = c + di
  const float c = Y.GetReal ();
  const float d = Y.GetImaginary ();

  // X = mag(X)e^(i*arg(X))
  const float magX = X.GetMagnitude ();
  const float argX = X.GetArgument ();

  // calculate result in polar form
  const float Mag = pow (magX, c) / exp (d * argX);
  const float Arg = d * log (magX) + c * argX;
  ComplexNumber result (Mag, Arg, POLAR);
  return result;
}

ComplexNumber rootC (const ComplexNumber &X, const ComplexNumber &Y) {
  // takes the complex root Y of complex number X
  ComplexNumber result;
  ComplexNumber root = ComplexNumber (1) / Y;
  result = powC (X, root);
  return result;
}

ComplexNumber BinetFibFormula (const ComplexNumber &n) {
  // calculate the nth term of the fib sequence using a complex number as n
  return (powC (ComplexNumber (PHI), n) - (powC (ComplexNumber (-1 / PHI), n))) / sqrt (5.0f);
}
