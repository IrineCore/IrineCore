USE [dnmembership]
GO
/****** Object:  Table [dbo].[EventScheduleForPetal]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[EventScheduleForPetal]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[EventScheduleForPetal](
	[EventScheduleID] [int] IDENTITY(1,1) NOT NULL,
	[StartDate] [smalldatetime] NOT NULL,
	[EndDate] [smalldatetime] NOT NULL,
	[PetalAmount] [int] NOT NULL,
	[EventConditionID] [int] NULL,
 CONSTRAINT [PK_EventScheduleForPetal] PRIMARY KEY CLUSTERED 
(
	[EventScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventConditionForPetal_EventScheduleForPetal_EventConditionID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventScheduleForPetal]'))
ALTER TABLE [dbo].[EventScheduleForPetal]  WITH CHECK ADD  CONSTRAINT [FK_EventConditionForPetal_EventScheduleForPetal_EventConditionID] FOREIGN KEY([EventConditionID])
REFERENCES [dbo].[EventConditionForPetal] ([EventConditionID])
GO
IF  EXISTS (SELECT * FROM sys.foreign_keys WHERE object_id = OBJECT_ID(N'[dbo].[FK_EventConditionForPetal_EventScheduleForPetal_EventConditionID]') AND parent_object_id = OBJECT_ID(N'[dbo].[EventScheduleForPetal]'))
ALTER TABLE [dbo].[EventScheduleForPetal] CHECK CONSTRAINT [FK_EventConditionForPetal_EventScheduleForPetal_EventConditionID]
GO
