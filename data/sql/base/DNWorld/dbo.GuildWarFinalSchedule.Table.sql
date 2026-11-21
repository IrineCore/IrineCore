USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildWarFinalSchedule]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWarFinalSchedule](
	[GuildWarScheduleID] [smallint] NOT NULL,
	[MatchTypeCode] [tinyint] NOT NULL,
	[MatchStartTime] [smalldatetime] NOT NULL,
	[MatchEndTime] [smalldatetime] NOT NULL,
	[RegisterDate] [datetime] NOT NULL,
	[ChangeDate] [datetime] NOT NULL,
 CONSTRAINT [PK_GuildWarFinalSchedule] PRIMARY KEY CLUSTERED 
(
	[GuildWarScheduleID] ASC,
	[MatchTypeCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
