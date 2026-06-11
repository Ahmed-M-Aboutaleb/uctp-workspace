"use client"

import { useState } from "react"
import { useForm } from "react-hook-form"
import { toast } from "react-toastify"
import { 
  User, BadgeCheck, ShieldCheck, Briefcase, 
  Clock, Building2, ChevronDown, Loader2, CheckCircle2, Save 
} from "lucide-react"
import { UpdateProfile } from "@/ServerActions/Profile/UpdateProfile"
import { AdminResponse, UpdateProfileData } from "@/Types/AdminTypes"

export default function AdminProfile({ profile }: { profile: AdminResponse }) {
  const [isSubmitting, setIsSubmitting] = useState(false)
  const [saveSuccess, setSaveSuccess] = useState(false)
  const [displayName, setDisplayName] = useState(profile.full_name)
  const [displayTitle, setDisplayTitle] = useState(profile.title)

  const { register, handleSubmit, formState: { errors } } = useForm<UpdateProfileData>({
    defaultValues: {
      full_name: profile.full_name,
      title: profile.title,
      department: profile.department,
      semester: "Spring 2026"
    }
  })

  const onSubmit = async (formData: UpdateProfileData) => {
    setIsSubmitting(true)
    setSaveSuccess(false)
    try {
      const newProfile = await UpdateProfile(formData);
      setDisplayName(newProfile.full_name)
      setDisplayTitle(newProfile.title)
      setSaveSuccess(true)
      toast.success("Profile updated successfully")
    } catch (error: any) {
      toast.error(error.message || "Failed to save changes")
    } finally {
      setIsSubmitting(false)
      setTimeout(() => setSaveSuccess(false), 3000)
    }
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="relative overflow-hidden rounded-3xl bg-white border border-slate-200 shadow-xl shadow-blue-100/40">
        <div className="h-24 bg-gradient-to-r from-indigo-600 to-violet-600" />
        <div className="px-6 pb-6">
          <div className="relative -mt-10 flex items-center gap-4">
            <div className="flex h-20 w-20 items-center justify-center rounded-2xl bg-white p-1.5 shadow-lg">
              <div className="flex h-full w-full items-center justify-center rounded-xl bg-slate-50 text-indigo-600">
                <User size={32} />
              </div>
            </div>
            <div className="pt-10">
              <div className="flex items-center gap-2">
                <h1 className="text-xl font-bold text-slate-900">{displayName}</h1>
                <BadgeCheck className="text-blue-500 h-5 w-5" />
              </div>
              <p className="text-xs text-slate-500 font-medium">{displayTitle} • Admin Access</p>
            </div>
          </div>
        </div>
      </div>

      {/* Form */}
      <div className="rounded-3xl border border-slate-200 bg-white shadow-sm overflow-hidden">
        <div className="border-b border-slate-100 bg-slate-50/50 px-6 py-4 flex items-center justify-between">
          <h2 className="font-bold text-slate-800">System Administrator Settings</h2>
          <ShieldCheck className="h-5 w-5 text-indigo-600" />
        </div>
        <form onSubmit={handleSubmit(onSubmit)} className="p-6 space-y-5">
          <div className="space-y-1.5">
            <label className="text-[11px] font-bold text-slate-500 uppercase ml-1">Full Name</label>
            <input {...register("full_name", { required: true })} className="w-full rounded-2xl border border-slate-200 p-3 text-sm focus:border-indigo-500 outline-none" />
          </div>
          <div className="grid grid-cols-2 gap-4">
             <div className="space-y-1.5">
                <label className="text-[11px] font-bold text-slate-500 uppercase ml-1">Title</label>
                <input {...register("title")} className="w-full rounded-2xl border border-slate-200 p-3 text-sm focus:border-indigo-500 outline-none" />
             </div>
             <div className="space-y-1.5">
                <label className="text-[11px] font-bold text-slate-500 uppercase ml-1">Semester</label>
                <input {...register("semester")} className="w-full rounded-2xl border border-slate-200 p-3 text-sm focus:border-indigo-500 outline-none" />
             </div>
          </div>
          <button type="submit" disabled={isSubmitting} className="w-full bg-indigo-600 text-white rounded-2xl p-4 font-bold transition-all hover:bg-indigo-700 disabled:opacity-50">
            {isSubmitting ? "Saving..." : saveSuccess ? "Changes Saved!" : "Save Profile"}
          </button>
        </form>
      </div>
    </div>
  )
}