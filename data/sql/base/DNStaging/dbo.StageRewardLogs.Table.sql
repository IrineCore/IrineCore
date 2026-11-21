USE [dnstaging]
GO
/****** Object:  Table [dbo].[StageRewardLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageRewardLogs](
	[StageStartDate] [datetime2](0) NOT NULL,
	[StageEndDate] [datetime2](0) NULL,
	[StageStartID] [bigint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[RewardBoxCode] [tinyint] NULL,
	[RewardItemSerial] [bigint] NULL,
	[RewardItemID] [int] NULL,
	[WorldID] [int] NULL,
	[RewardFailReason] [int] NULL
) ON [PRIMARY]
GO
