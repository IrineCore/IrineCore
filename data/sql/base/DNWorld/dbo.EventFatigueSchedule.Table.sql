USE [dnworld]
GO
/****** Object:  Table [dbo].[EventFatigueSchedule]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[EventFatigueSchedule](
	[EventFatigueScheduleID] [int] IDENTITY(1,1) NOT NULL,
	[GiveDate] [datetime] NOT NULL,
	[FatigueValue] [int] NOT NULL,
	[GiveFlag] [bit] NOT NULL,
 CONSTRAINT [PK_EventFatigueSchedule] PRIMARY KEY CLUSTERED 
(
	[EventFatigueScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[EventFatigueSchedule] ADD  CONSTRAINT [DF_EventFatigueSchedule_GiveFlag]  DEFAULT ((0)) FOR [GiveFlag]
GO
