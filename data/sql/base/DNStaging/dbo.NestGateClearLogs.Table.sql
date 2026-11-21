USE [dnstaging]
GO
/****** Object:  Table [dbo].[NestGateClearLogs]    Script Date: 11/14/2026 9:00:09 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NestGateClearLogs](
	[NestGateStartDate] [datetime2](0) NOT NULL,
	[NestGateEndDate] [datetime2](0) NULL,
	[NestGateStartID] [bigint] NOT NULL,
	[StageStartID] [bigint] NULL,
	[CharacterID] [bigint] NOT NULL,
	[JobCode] [tinyint] NOT NULL,
	[CharacterLevel] [tinyint] NOT NULL,
	[ClearFlag] [bit] NOT NULL,
	[NestRound] [tinyint] NOT NULL,
	[StartPartySize] [tinyint] NOT NULL,
	[EndPartySize] [tinyint] NULL,
	[MapID] [int] NOT NULL,
	[WorldID] [int] NOT NULL
) ON [PRIMARY]
GO
