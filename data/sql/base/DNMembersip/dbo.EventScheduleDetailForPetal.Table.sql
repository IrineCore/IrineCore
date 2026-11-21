USE [dnmembership]
GO
/****** Object:  Table [dbo].[EventScheduleDetailForPetal]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[EventScheduleDetailForPetal]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[EventScheduleDetailForPetal](
	[EventScheduleID] [int] NOT NULL,
	[AccountID] [int] NOT NULL,
	[PetalIncomeID] [bigint] NULL,
	[RegisterDate] [datetime2](0) NULL,
	[ConsecutionBonusPetalIncomeID] [bigint] NULL,
 CONSTRAINT [PK_EventScheduleDetailForPetal] PRIMARY KEY CLUSTERED 
(
	[EventScheduleID] ASC,
	[AccountID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventScheduleForPetal_EventScheduleDetailForPetal_EventScheduleID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventScheduleDetailForPetal]'))
ALTER TABLE [dbo].[EventScheduleDetailForPetal]  WITH CHECK ADD  CONSTRAINT [FK_EventScheduleForPetal_EventScheduleDetailForPetal_EventScheduleID] FOREIGN KEY([EventScheduleID])
REFERENCES [dbo].[EventScheduleForPetal] ([EventScheduleID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventScheduleForPetal_EventScheduleDetailForPetal_EventScheduleID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventScheduleDetailForPetal]'))
ALTER TABLE [dbo].[EventScheduleDetailForPetal] CHECK CONSTRAINT [FK_EventScheduleForPetal_EventScheduleDetailForPetal_EventScheduleID]
GO
