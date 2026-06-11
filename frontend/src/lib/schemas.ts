import { z } from 'zod'

export const featureSchema = z.object({
  name: z.string().min(2, 'Name must be at least 2 characters'),
  description: z.string().min(5, 'Description must be at least 5 characters'),
  status: z.enum(['active', 'inactive']),
})

export const roomSchema = z.object({
  name: z.string().min(2, 'Room name must be at least 2 characters'),
  capacity: z.coerce.number().min(1, 'Capacity must be at least 1'),
  location: z.string().min(3, 'Location must be at least 3 characters'),
  status: z.enum(['available', 'occupied']),
})

export const instructorSchema = z.object({
  name: z.string().min(2, 'Name must be at least 2 characters'),
  email: z.string().email('Please enter a valid email'),
  phone: z.string().min(10, 'Phone must be at least 10 characters'),
  specialization: z.string().min(2, 'Specialization must be at least 2 characters'),
  status: z.enum(['active', 'inactive']),
})

export const studentSchema = z.object({
  name: z.string().min(2, 'Name must be at least 2 characters'),
  email: z.string().email('Please enter a valid email'),
  phone: z.string().min(10, 'Phone must be at least 10 characters'),
  enrollmentDate: z.string().min(1, 'Enrollment date is required'),
  status: z.enum(['active', 'graduated', 'inactive']),
})

export type FeatureFormData = z.infer<typeof featureSchema>
export type RoomFormData = z.infer<typeof roomSchema>
export type InstructorFormData = z.infer<typeof instructorSchema>
export type StudentFormData = z.infer<typeof studentSchema>
