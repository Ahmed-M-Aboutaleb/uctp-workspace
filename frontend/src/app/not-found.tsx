"use client"

import Link from "next/link"
import { 
  BookOpen, 
  Search, 
  Home, 
  ArrowLeft, 
  FileQuestion,
  GraduationCap
} from "lucide-react"

export default function NotFound() {
  return (
    <div className="flex min-h-screen flex-col items-center justify-center bg-background px-6 py-24 text-center animate-in fade-in duration-700">
      {/* Decorative Background Elements */}
      <div className="absolute inset-0 -z-10 overflow-hidden">
        <div className="absolute -top-[10%] left-[10%] h-64 w-64 rounded-full bg-indigo-600/5 blur-3xl" />
        <div className="absolute bottom-[10%] right-[10%] h-96 w-96 rounded-full bg-indigo-500/5 blur-3xl" />
      </div>

      {/* Main Illustration/Icon Section */}
      <div className="relative mb-8">
        <div className="flex h-32 w-32 items-center justify-center rounded-3xl bg-muted/50 ring-1 ring-border shadow-inner">
          <FileQuestion className="h-16 w-16 text-muted-foreground animate-pulse" />
        </div>
        <div className="absolute -bottom-2 -right-2 flex h-12 w-12 items-center justify-center rounded-2xl bg-indigo-600 shadow-lg ring-4 ring-background">
          <Search className="h-6 w-6 text-white" />
        </div>
      </div>

      {/* Text Content */}
      <div className="max-w-md space-y-4">
        <h1 className="text-7xl font-black tracking-tighter text-foreground md:text-8xl">
          4<span className="text-indigo-600">0</span>4
        </h1>
        <h2 className="text-2xl font-bold tracking-tight text-foreground md:text-3xl">
          Lesson Not Found
        </h2>
        <p className="text-muted-foreground">
          Sorry, the page you are looking for seems to have been removed, renamed, or perhaps never existed in our curriculum.
        </p>
      </div>

      {/* Action Buttons */}
      <div className="mt-10 flex flex-col items-center gap-4 sm:flex-row">
        <button 
          onClick={() => window.history.back()}
          className="flex w-full items-center justify-center gap-2 rounded-xl border border-border bg-card px-6 py-3 text-sm font-bold transition-all hover:bg-muted active:scale-95 sm:w-auto"
        >
          <ArrowLeft size={18} />
          Go Back
        </button>
        
        <Link 
          href="/"
          className="flex w-full items-center justify-center gap-2 rounded-xl bg-indigo-600 px-6 py-3 text-sm font-bold text-white shadow-lg shadow-indigo-600/20 transition-all hover:opacity-90 active:scale-95 sm:w-auto"
        >
          <Home size={18} />
          Return Dashboard
        </Link>
      </div>


    </div>
  )
}