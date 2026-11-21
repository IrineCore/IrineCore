USE [dnworld]
GO
/****** Object:  Table [dbo].[GuildWarSchedule]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GuildWarSchedule](
	[GuildWarScheduleID] [smallint] IDENTITY(1,1) NOT NULL,
	[ApplyStartDate] [smalldatetime] NOT NULL,
	[ApplyEndDate] [smalldatetime] NOT NULL,
	[OpeningStartDate] [smalldatetime] NOT NULL,
	[OpeningEndDate] [smalldatetime] NOT NULL,
	[RewardStartDate] [smalldatetime] NOT NULL,
	[RewardEndDate] [smalldatetime] NOT NULL,
	[WinnersWeightRate] [smallint] NOT NULL,
	[WinningTeamColorCode] [tinyint] NULL,
	[RegisterDate] [datetime] NOT NULL,
	[ChangeDate] [datetime] NOT NULL,
	[FinalProgressFlag] [bit] NOT NULL,
	[RewardExpirationDate] [smalldatetime] NULL,
 CONSTRAINT [PK_GuildWarSchedule] PRIMARY KEY CLUSTERED 
(
	[GuildWarScheduleID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
ALTER TABLE [dbo].[GuildWarSchedule] ADD  CONSTRAINT [DF_GuildWarSchedule_FinalProgressFlag]  DEFAULT ((1)) FOR [FinalProgressFlag]
GO
