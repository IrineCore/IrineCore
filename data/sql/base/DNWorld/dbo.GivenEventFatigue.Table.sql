USE [dnworld]
GO
/****** Object:  Table [dbo].[GivenEventFatigue]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GivenEventFatigue](
	[CharacterID] [bigint] NOT NULL,
	[EventFatigueScheduleID] [int] NOT NULL,
	[GivenDate] [datetime] NOT NULL,
 CONSTRAINT [PK_GivenEventFatigue] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC,
	[EventFatigueScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
