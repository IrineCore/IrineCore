USE [dnstaging]
GO
/****** Object:  Table [dbo].[NestGateClearLogs_20130920]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[NestGateClearLogs_20130920](
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
/****** Object:  Index [IX_NC_NestGateClearLogs_20130920_NestGateStartDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_NestGateClearLogs_20130920_NestGateStartDate] ON [dbo].[NestGateClearLogs_20130920]
(
	[NestGateStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_NestGateClearLogs_20130920_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_NestGateClearLogs_20130920_CharacterID] ON [dbo].[NestGateClearLogs_20130920]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[NestGateClearLogs_20130920]  WITH CHECK ADD  CONSTRAINT [CK_NestGateClearLogs_20130920_NestGateStartDate] CHECK  (([NestGateStartDate]>='2013-09-20' AND [NestGateStartDate]<'2013-09-21'))
GO
ALTER TABLE [dbo].[NestGateClearLogs_20130920] CHECK CONSTRAINT [CK_NestGateClearLogs_20130920_NestGateStartDate]
GO
