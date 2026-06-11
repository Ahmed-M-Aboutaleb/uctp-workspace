"use client"

import { InstructorRecord } from "@/Types/StudentTypes"
import { GraduationCap, User } from "lucide-react"

export default function InstructorProfile({ profile }: { profile: InstructorRecord }) {
  
  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="relative overflow-hidden rounded-3xl bg-white border border-slate-200 shadow-xl shadow-indigo-100/40">
        <div className="h-24 bg-gradient-to-r from-indigo-600 to-teal-500" />
        <div className="px-6 pb-6">
          <div className="relative -mt-10 flex items-center gap-4">
            <div className="h-20 w-20 rounded-2xl bg-white p-1.5 shadow-lg">
              <div className="flex h-full w-full items-center justify-center rounded-xl bg-slate-50 text-indigo-600">
                <User size={32} />
              </div>
            </div>
            <div className="pt-10">
              <div className="flex items-center gap-2">
                <h1 className="text-xl font-bold text-slate-900">{profile.name.toUpperCase()}</h1>
                <GraduationCap className="text-indigo-500 h-5 w-5" />
              </div>
              <p className="text-xs text-slate-500 font-medium">Instructor ID: {profile.user_id}</p>
            </div>
          </div>
        </div>
      </div>

    </div>
  )
}

function InfoCard({ icon, label, value }: { icon: React.ReactNode, label: string, value: string }) {
  return (
    <div className="p-5 rounded-3xl bg-white border border-slate-100 shadow-sm flex items-start gap-4">
      <div className="p-2 rounded-xl bg-slate-50">{icon}</div>
      <div>
        <p className="text-[10px] font-bold text-slate-400 uppercase tracking-widest">{label}</p>
        <p className="text-sm font-semibold text-slate-700">{value}</p>
      </div>
    </div>
  )
}