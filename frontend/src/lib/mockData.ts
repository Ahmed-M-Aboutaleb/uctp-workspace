'use client'

import { useState } from 'react'

export interface Feature {
  id: string
  name: string
  description: string
  status: 'active' | 'inactive'
  createdAt: string
}

export interface Room {
  id: string
  name: string
  capacity: number
  location: string
  status: 'available' | 'occupied'
  createdAt: string
}

export interface Instructor {
  id: string
  name: string
  email: string
  phone: string
  specialization: string
  status: 'active' | 'inactive'
  createdAt: string
}

export interface Student {
  id: string
  name: string
  email: string
  phone: string
  enrollmentDate: string
  status: 'active' | 'graduated' | 'inactive'
  createdAt: string
}

export interface Course {
  id: string
  name: string
  instructor: string
  description: string
  credits: number
  level: 'beginner' | 'intermediate' | 'advanced'
  status: 'active' | 'inactive'
  createdAt: string
}

// Mock data
const mockFeatures: Feature[] = [
  {
    id: '1',
    name: 'Video Streaming',
    description: 'Stream high-quality video content',
    status: 'active',
    createdAt: '2024-01-15',
  },
  {
    id: '2',
    name: 'Live Classes',
    description: 'Real-time interactive classes',
    status: 'active',
    createdAt: '2024-01-20',
  },
  {
    id: '3',
    name: 'Assignments',
    description: 'Submit and track assignments',
    status: 'active',
    createdAt: '2024-02-01',
  },
]

const mockRooms: Room[] = [
  {
    id: '1',
    name: 'Room 101',
    capacity: 30,
    location: 'Building A, Floor 1',
    status: 'available',
    createdAt: '2024-01-10',
  },
  {
    id: '2',
    name: 'Room 201',
    capacity: 50,
    location: 'Building B, Floor 2',
    status: 'occupied',
    createdAt: '2024-01-12',
  },
  {
    id: '3',
    name: 'Lab 301',
    capacity: 25,
    location: 'Building A, Floor 3',
    status: 'available',
    createdAt: '2024-01-14',
  },
]

const mockInstructors: Instructor[] = [
  {
    id: '1',
    name: 'Dr. John Smith',
    email: 'john@example.com',
    phone: '+1-555-0001',
    specialization: 'Computer Science',
    status: 'active',
    createdAt: '2023-12-01',
  },
  {
    id: '2',
    name: 'Prof. Sarah Johnson',
    email: 'sarah@example.com',
    phone: '+1-555-0002',
    specialization: 'Mathematics',
    status: 'active',
    createdAt: '2023-12-05',
  },
  {
    id: '3',
    name: 'Dr. Michael Brown',
    email: 'michael@example.com',
    phone: '+1-555-0003',
    specialization: 'Physics',
    status: 'inactive',
    createdAt: '2023-11-20',
  },
]

const mockStudents: Student[] = [
  {
    id: '1',
    name: 'Alice Johnson',
    email: 'alice@example.com',
    phone: '+1-555-0101',
    enrollmentDate: '2024-01-15',
    status: 'active',
    createdAt: '2024-01-15',
  },
  {
    id: '2',
    name: 'Bob Smith',
    email: 'bob@example.com',
    phone: '+1-555-0102',
    enrollmentDate: '2024-01-20',
    status: 'active',
    createdAt: '2024-01-20',
  },
  {
    id: '3',
    name: 'Carol White',
    email: 'carol@example.com',
    phone: '+1-555-0103',
    enrollmentDate: '2023-08-01',
    status: 'graduated',
    createdAt: '2023-08-01',
  },
]

export const mockDataStore = {
  features: mockFeatures,
  rooms: mockRooms,
  instructors: mockInstructors,
  students: mockStudents,
}

export function useMockData<T extends Feature | Room | Instructor | Student>(
  initialData: T[]
) {
  const [data, setData] = useState<T[]>(initialData)
  const [searchQuery, setSearchQuery] = useState('')

  const filteredData = data.filter((item) => {
    const searchStr = searchQuery.toLowerCase()
    return JSON.stringify(item).toLowerCase().includes(searchStr)
  })

  const addItem = (newItem: Omit<T, 'id' | 'createdAt'>) => {
    const item = {
      ...newItem,
      id: Date.now().toString(),
      createdAt: new Date().toISOString().split('T')[0],
    } as T
    setData([...data, item])
    return item
  }

  const updateItem = (id: string, updates: Partial<T>) => {
    setData(data.map((item) => (item.id === id ? { ...item, ...updates } : item)))
  }

  const deleteItem = (id: string) => {
    setData(data.filter((item) => item.id !== id))
  }

  return {
    data: filteredData,
    allData: data,
    setSearchQuery,
    searchQuery,
    addItem,
    updateItem,
    deleteItem,
  }
}
