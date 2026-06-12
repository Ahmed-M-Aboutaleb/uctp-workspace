'use client'

import { useUserData } from '@/context/UserData'
import { removeToken, setToken } from '@/cookies/auth.actions'
import { LoginSchema, LoginType } from '@/schema/AuthScheema'
import { zodResolver } from '@hookform/resolvers/zod'
import { ArrowRight, Eye, EyeOff, GraduationCap, IdCard, Lock } from 'lucide-react'
import { useRouter } from 'next/navigation'
import { useEffect, useState } from 'react'
import { useForm } from 'react-hook-form'
import { toast } from 'react-toastify'
import { Bounce } from "react-toastify/unstyled"

export default function LoginPage() {
  const [showPassword, setShowPassword] = useState(false);
  const [isLoading, setIsLoading] = useState(false);
  const { setUser } = useUserData();
  const router = useRouter();


  const { register, handleSubmit, reset, setError, formState } = useForm<LoginType>({
    resolver: zodResolver(LoginSchema),
    defaultValues: {
      username: '',
      password: ''
    },
    mode: 'onSubmit'
  });

  useEffect(() => {
    removeToken();
  }, []);


  async function onSubmit(data: LoginType) {
    setIsLoading(true);
    const formData = new URLSearchParams();
    formData.append('username', data.username);
    formData.append('password', data.password);
    try {
      const res = await fetch(`${process.env.NEXT_PUBLIC_API_URL}/api/auth/login`, {
        method: 'post',
        headers: { "Content-Type": "application/x-www-form-urlencoded" },
        body: formData,
      });
      const result = await res.json();
      if (result.access_token) {
        toast.success('Account logged in successfully!', {
          position: "top-right",
          autoClose: 2000,
          hideProgressBar: false,
          closeOnClick: false,
          pauseOnHover: true,
          draggable: true,
          progress: undefined,
          theme: "light",
          transition: Bounce,
        });

        setUser({
          token: result.access_token,
          role: result.user.role,
          email: result.user.email,
          profile: result.user.profile
        });
        await setToken(result.access_token);
        localStorage.setItem('role', result.user.role);
        localStorage.setItem('email', result.user.email);
        reset();
        setTimeout(() => {
          if (result.user.role === "admin") {
            router.push("/admin");
          } else if (result.user.role === "instructor") {
            router.push("/instructor")
          } else if (result.user.role === "student") {
            router.push("/student")
          }
        }, 2000);
      } else {
        throw new Error(result.detail || "Login failed");
      }
    } catch (err: any) {
      setError('password', { message: err.message || "Invalid email or password" });
    } finally {
      setIsLoading(false);
    }
  }


  return (
    <div className="min-h-screen flex flex-col lg:flex-row">
      {/* Left Side - Gradient Background */}
      <div className="relative lg:bg-gradient-to-br bg-[#F5F5F7] lg:from-[#5B6EE1] lg:via-[#5B6EE1] lg:to-[#7B5EA7] lg:w-1/2 flex flex-col">
        {/* Logo - Desktop */}
        <div className="hidden lg:flex items-center gap-3 p-8">
          <div className="w-10 h-10 bg-white/20 rounded-lg flex items-center justify-center">
            <GraduationCap className="w-6 h-6 text-white" />
          </div>
          <span className="text-white text-xl font-semibold">Capital University</span>
        </div>

        {/* Mobile Header */}
        <div className="lg:hidden text-white flex flex-col items-center pt-12 pb-8  px-6">
          <div className="w-24 h-24 bg-[#4A5BC7] rounded-full flex items-center justify-center mb-6">
            <GraduationCap className="w-12 h-12 " />
          </div>
          <h1 className=" text-3xl font-bold text-center text-[#4A5BC7]">Capital University</h1>
          <p className=" text-lg italic mt-2 text-[#4A5BC7]">Your Future Starts Here</p>
        </div>

        {/* Desktop Bottom Content */}
        <div className="hidden lg:flex flex-col justify-end flex-1 p-8 pb-16">
          <h2 className="text-white text-5xl font-bold leading-tight">
            Your Future
            <br />
            Starts Here
          </h2>
          <p className="text-white/80 mt-4 max-w-md">
            Access your academic dashboard, connect with faculty, and manage your university journey securely.
          </p>

          {/* Decorative Grid Pattern */}
          <div className="mt-8 flex gap-4">
            <div className="grid grid-cols-5 ">
              <div className="w-10 h-10 bg-white/20 " />
              <div className="w-10 h-10  " />
              <div className="w-10 h-10 bg-white/20 " />
              <div className="w-10 h-10  " />
              <div className="w-10 h-10  bg-white/20" />
              <div className="w-10 h-10  " />
              <div className="w-10 h-10  bg-white/10" />
              <div className="w-10 h-10  " />
              <div className="w-10 h-10 bg-white/10" />
              <div className="w-10 h-10  " />
              <div className="w-10 h-10  bg-white/20" />
              <div className="w-10 h-10  " />
              <div className="w-10 h-10  bg-white/20" />
              <div className="w-10 h-10  " />
              <div className="w-10 h-10  bg-white/20" />
            </div>
          </div>
        </div>
      </div>

      <div className="flex-1 bg-[#F5F5F7] lg:bg-white  flex items-start lg:items-center justify-center px-6 py-8 lg:py-0">
        <div className="w-full max-w-md">
          {/* Login Card */}
          <div className="bg-white rounded-3xl lg:rounded-2xl shadow-sm lg:shadow-none p-8 lg:p-0">
            <h2 className="text-2xl lg:text-3xl font-bold text-gray-900 mb-2">Welcome Back</h2>
            <p className="text-gray-500 mb-8">
              <span className="lg:hidden">Please enter your credentials to access the portal.</span>
              <span className="hidden lg:inline">Sign in to access your student portal.</span>
            </p>

            <form onSubmit={handleSubmit(onSubmit)} className="space-y-6">
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2 uppercase tracking-wide">
                  Student ID
                </label>
                <div className="relative">
                  <div className="absolute inset-y-0 left-0 pl-4 flex items-center pointer-events-none">
                    <IdCard className="h-5 w-5 text-gray-400" />
                  </div>
                  <input
                    type="text"
                    {...register('username')}
                    placeholder="e.g. 12345678"
                    className="w-full pl-12 pr-4 py-4 bg-gray-50 border border-gray-200 rounded-xl text-gray-900 placeholder:text-gray-400 focus:outline-none focus:ring-2 focus:ring-[#5B6EE1] focus:border-transparent transition-all"
                  />
                </div>
                {formState.errors.username && (
                  <p className="text-red-500 text-sm mt-1">
                    {formState.errors.username.message}
                  </p>
                )}
              </div>

              {/* Password Field */}
              <div>
                <label className="block text-sm font-medium text-gray-700 mb-2 uppercase tracking-wide">
                  Password
                </label>
                <div className="relative">
                  <div className="absolute inset-y-0 left-0 pl-4 flex items-center pointer-events-none">
                    <Lock className="h-5 w-5 text-gray-400" />
                  </div>
                  <input
                    type={showPassword ? "text" : "password"}
                    {...register('password')}
                    placeholder="••••••••"
                    className="w-full pl-12 pr-4 py-4 bg-gray-50 border border-gray-200 rounded-xl text-gray-900 placeholder:text-gray-400 focus:outline-none focus:ring-2 focus:ring-[#5B6EE1] focus:border-transparent transition-all"
                  />
                  <button
                    type="button" // Important: set to button so it doesn't trigger form submit
                    onClick={() => setShowPassword(!showPassword)}
                    className="absolute inset-y-0 right-0 pr-4 flex items-center text-gray-400 hover:text-gray-600 transition-colors"
                  >
                    {showPassword ? (
                      <EyeOff className="h-5 w-5" />
                    ) : (
                      <Eye className="h-5 w-5" />
                    )}
                  </button>
                </div>
                {formState.errors.password && (
                  <p className="text-red-500 text-sm mt-1">
                    {formState.errors.password.message}
                  </p>
                )}
              </div>

              <button
                type="submit"
                className="w-full cursor-pointer group bg-[#4A5BC7] hover:bg-[#5B6EE1] text-white font-semibold py-4 px-6 rounded-xl flex items-center justify-center gap-2 transition-colors"
              >
                <span className="uppercase tracking-wide">Log In</span>
                <ArrowRight className="w-5 h-5 group-hover:translate-x-2 transition-transform" />
              </button>
            </form>

            <p className="hidden lg:block text-center text-gray-400 text-sm mt-8">
              Protected by Capital University IT Services
            </p>
          </div>
        </div>
      </div>
    </div>
  )
}
