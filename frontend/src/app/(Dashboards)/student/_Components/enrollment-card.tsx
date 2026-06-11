// components/enrollment-card.tsx
'use client';

import React, { useTransition } from 'react';
import { Course } from '@/Types/CourseTypes';
import { User, Users, Tag, CheckCircle, Loader2, MinusCircle, PlusCircle } from 'lucide-react';
import { UnEnrollStudent } from '@/ServerActions/Enrollment/UnEnrollStudent';
import { EnrollStudent } from '@/ServerActions/Enrollment/EnrollStudent';
import { toast } from 'react-toastify';

interface Props {
  course: Course;
  currentStudentId: number;
}

export default function EnrollmentCard({ course, currentStudentId }: Props) {
  const [isPending, startTransition] = useTransition();
    
  const isEnrolled = course.students.some((s) => s.student_id === currentStudentId);

  const handleToggleEnrollment = () => {
    startTransition(async () => {
      try {
        if (isEnrolled) {
          await UnEnrollStudent(course.course_id);
        } else {
          await EnrollStudent(course.course_id);
        }
      } catch (error) {
        toast.error('Enrollment for this course is currently closed.')
    }
    });
  };

  return (
    <div className={`group relative flex flex-col overflow-hidden rounded-2xl border transition-all duration-300 ${
      isEnrolled ? 'border-indigo-200 bg-indigo-50/20' : 'border-slate-200 bg-white'
    }`}>
      <div className="flex flex-1 flex-col p-6">
        <div className="mb-4 flex items-start justify-between">
          <div className="space-y-1">
            <h3 className="text-lg font-bold text-slate-900 leading-tight">
              {course.course_name}
            </h3>
            {isEnrolled && (
              <span className="flex items-center gap-1 text-[10px] font-black uppercase text-indigo-600 tracking-wider">
                <CheckCircle size={12} strokeWidth={3} /> Enrolled
              </span>
            )}
          </div>
        </div>

        <div className="mb-6 space-y-3">
          <div className="flex items-center gap-2.5 text-sm text-slate-600">
            <User size={14} className="text-slate-400" />
            <span className="font-medium truncate">
              {course.instructors.length > 0 ? course.instructors[0].name : 'Staff'}
            </span>
          </div>
          <div className="flex items-center gap-2.5 text-sm text-slate-600">
            <Users size={14} className="text-slate-400" />
            <span>{course.students.length} Students</span>
          </div>
        </div>

        <div className="mb-8 flex flex-wrap gap-1.5">
          {course.features.map((feat) => (
            <span key={feat.feature_id} className="rounded-md bg-slate-100 px-2 py-0.5 text-[10px] font-bold text-slate-500 uppercase">
              {feat.feature_name}
            </span>
          ))}
        </div>

        <button
          onClick={handleToggleEnrollment}
          disabled={isPending}
          className={`mt-auto flex w-full items-center justify-center gap-2 rounded-xl py-3 text-sm font-bold transition-all disabled:opacity-50 ${
            isEnrolled
              ? 'bg-white border border-red-200 text-red-600 hover:bg-red-50'
              : 'bg-indigo-700 text-white hover:bg-indigo-600'
          }`}
        >
          {isPending ? (
            <Loader2 className="animate-spin" size={18} />
          ) : isEnrolled ? (
            <><MinusCircle size={18} /> Unenroll</>
          ) : (
            <><PlusCircle size={18} /> Enroll Now</>
          )}
        </button>
      </div>
    </div>
  );
}