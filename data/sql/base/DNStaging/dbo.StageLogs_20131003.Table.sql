USE [dnstaging]
GO
/****** Object:  Table [dbo].[StageLogs_20131003]    Script Date: 11/14/2026 9:00:07 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[StageLogs_20131003](
	[StageStartDate] [datetime2](0) NOT NULL,
	[StageEndDate] [datetime2](0) NULL,
	[StageStartID] [bigint] NOT NULL,
	[PartyStartID] [bigint] NULL,
	[PartySize] [tinyint] NOT NULL,
	[MapID] [int] NOT NULL,
	[DifficultyCode] [tinyint] NOT NULL,
	[WhereToGoCode] [tinyint] NULL,
	[WorldID] [int] NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_StageLogs_20131003_StageStartDate]    Script Date: 11/14/2026 9:00:07 AM ******/
CREATE CLUSTERED INDEX [IX_NC_StageLogs_20131003_StageStartDate] ON [dbo].[StageLogs_20131003]
(
	[StageStartDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_StageLogs_20131003_PartyStartID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_StageLogs_20131003_PartyStartID] ON [dbo].[StageLogs_20131003]
(
	[PartyStartID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_StageLogs_20131003_StageStartID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_StageLogs_20131003_StageStartID] ON [dbo].[StageLogs_20131003]
(
	[StageStartID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[StageLogs_20131003]  WITH CHECK ADD  CONSTRAINT [CK_StageLogs_20131003_StageStartDate] CHECK  (([StageStartDate]>='2013-10-03' AND [StageStartDate]<'2013-10-04'))
GO
ALTER TABLE [dbo].[StageLogs_20131003] CHECK CONSTRAINT [CK_StageLogs_20131003_StageStartDate]
GO
