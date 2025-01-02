#ifndef DFT_H
#define DFT_H

template <typename T>
void dft(std::vector<std::complex<T>> &in, std::vector<std::complex<T>> &out)
{
	out.resize(in.size());
	std::fill(out.begin(), out.end(), std::complex<T>(0, 0));
	for (size_t i = 0; i < out.size(); ++i)
	{
		T base = 2 * M_PI * i / in.size();
		T tmp = 0;
		for (size_t j = 0; j < in.size(); ++j)
		{
			out[i].real(out[i].real() + in[j].real() * fn_cos(tmp));
			out[i].imag(out[i].imag() + in[j].real() * fn_sin(tmp));
			tmp += base;
		}
	}
}

template <typename T>
void idft(std::vector<std::complex<T>> &in, std::vector<std::complex<T>> &out)
{
	out.resize(in.size());
	std::fill(out.begin(), out.end(), std::complex<T>(0, 0));
	for (size_t i = 0; i < out.size(); ++i)
	{
		out[i].real(in[0].real() * cos(0) + in[0].imag() * sin(0));
		T base = 2 * M_PI * i / in.size();
		T tmp = 0;
		for (size_t j = 1; j < in.size() / 2; ++j)
		{
			tmp += base;
			out[i].real(out[i].real() + 2 * (in[j].real() * fn_cos(tmp) + in[j].imag() * fn_sin(tmp)));
		}
		{
			tmp = M_PI * i;
			out[i].real(out[i].real() + in[in.size() / 2].real() * fn_cos(tmp) + in[in.size() / 2].imag() * fn_sin(tmp));
		}
		out[i].real(out[i].real() / in.size());
	}
}

template <typename T>
void dct(std::vector<std::complex<T>> &in, std::vector<std::complex<T>> &out)
{
	out.resize(in.size());
	std::fill(out.begin(), out.end(), std::complex<T>(0, 0));
	T c1 = M_PI / out.size();
	for (size_t i = 0; i < out.size(); ++i)
	{
		for (size_t j = 0; j < in.size(); ++j)
		{
			out[i].real(out[i].real() + in[j].real() * fn_cos(c1 * (j + .5) * i));
		}
	}
}

template <typename T>
void dct(std::vector<T> &in, std::vector<T> &out)
{
	out.resize(in.size());
	std::fill(out.begin(), out.end(), 0);
	T c1 = M_PI / out.size();
	for (size_t i = 0; i < out.size(); ++i)
	{
		for (size_t j = 0; j < in.size(); ++j)
		{
			out[i] += in[j] * fn_cos(c1 * (j + .5) * i);
		}
	}
}

template <typename T>
void idct(std::vector<std::complex<T>> &in, std::vector<std::complex<T>> &out)
{
	out.resize(in.size());
	T c1 = M_PI / in.size();
	for (size_t i = 0; i < out.size(); ++i)
	{
		out[i].real(.5 * in[0].real());
		for (size_t j = 1; j < in.size(); ++j)
		{
			out[i].real(out[i].real() + in[j].real() * fn_cos(c1 * j * (i + .5)));
		}
	}
}

template <typename T>
void idct(std::vector<T> &in, std::vector<T> &out)
{
	out.resize(in.size());
	T c1 = M_PI / in.size();
	for (size_t i = 0; i < out.size(); ++i)
	{
		out[i] = .5 * in[0];
		for (size_t j = 1; j < in.size(); ++j)
		{
			out[i] += in[j] * fn_cos(c1 * j * (i + .5));
		}
	}
}

template <typename T>
void mdct(std::vector<T> &in, std::vector<T> &out)
{
	out.resize(in.size() / 2);
	std::fill(out.begin(), out.end(), 0);
	T c1 = M_PI / out.size();
	T c2 = .5 + out.size() / 2;
	for (size_t i = 0; i < out.size(); ++i)
	{
		for (size_t j = 0; j < in.size(); ++j)
		{
			out[i] += in[j] * fn_cos(c1 * (j + c2) * (i + .5));
		}
	}
}

template <typename T>
void mdct(std::vector<std::complex<T>> &in, std::vector<std::complex<T>> &out)
{
	out.resize(in.size() / 2);
	std::fill(out.begin(), out.end(), std::complex<T>(0, 0));
	T c1 = M_PI / out.size();
	T c2 = .5 + out.size() / 2;
	for (size_t i = 0; i < out.size(); ++i)
	{
		for (size_t j = 0; j < in.size(); ++j)
		{
			out[i].real(out[i].real() + in[j].real() * fn_cos(c1 * (j + c2) * (i + .5)));
		}
	}
}

template <typename T>
void imdct(std::vector<T> &in, std::vector<T> &out)
{
	out.resize(in.size() * 2);
	std::fill(out.begin(), out.end(), 0);
	T c1 = M_PI / in.size();
	T c2 = .5 + in.size() / 2;
	for (size_t i = 0; i < out.size(); ++i)
	{
		for (size_t j = 0; j < in.size(); ++j)
		{
			out[i] += in[j] * fn_cos(c1 * (i + c2) * (j + .5));
		}
		out[i] /= in.size();
	}
}

template <typename T>
void imdct(std::vector<std::complex<T>> &in, std::vector<std::complex<T>> &out)
{
	out.resize(in.size() * 2);
	std::fill(out.begin(), out.end(), std::complex<T>(0, 0));
	T c1 = M_PI / in.size();
	T c2 = .5 + in.size() / 2;
	for (size_t i = 0; i < out.size(); ++i)
	{
		for (size_t j = 0; j < in.size(); ++j)
		{
			out[i].real(out[i].real() + in[j].real() * fn_cos(c1 * (i + c2) * (j + .5)));
		}
		out[i].real(out[i].real() / in.size());
	}
}

template <typename T>
void fft_rec(std::complex<T> *in, std::complex<T> *out, size_t size, size_t step)
{
	if (step >= size)
		return;
	fft_rec(out, in, size, step * 2);
	fft_rec(out + step, in + step, size, step * 2);
	T base = M_PI / size;
	T tmp = 0;
	for (size_t i = 0; i < size; i += step * 2)
	{
		T p = base * i;
		std::complex<T> t = std::complex<T>(fn_cos(p), fn_sin(p)) * out[step + i];
		in[i / 2] = out[i] + t;
		in[(i + size) / 2] = out[i] - t;
	}
}

template <typename T>
void fft(std::vector<std::complex<T>> &in, std::vector<std::complex<T>> &out)
{
	std::vector<std::complex<T>> tmp = in;
	out.resize(tmp.size());
	std::copy(tmp.begin(), tmp.end(), out.begin());
	fft_rec(&tmp[0], &out[0], out.size(), 1);
	out = tmp;
}

#endif
