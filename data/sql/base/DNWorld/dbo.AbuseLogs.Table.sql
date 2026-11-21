USE [dnworld]
GO
/****** Object:  Table [dbo].[AbuseLogs]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AbuseLogs](
	[LogDate] [datetime] NOT NULL,
	[AbuseCode] [tinyint] NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[LogMessage] [nvarchar](2000) COLLATE Chinese_PRC_CI_AS NOT NULL
) ON [DataFG1]
GO
/****** Object:  Index [IX_NC_AbuseLogs_LogDate]    Script Date: 11/14/2026 8:54:15 AM ******/
CREATE CLUSTERED INDEX [IX_NC_AbuseLogs_LogDate] ON [dbo].[AbuseLogs]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
/****** Object:  Index [IX_NN_AbuseLogs_CharacterID]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_AbuseLogs_CharacterID] ON [dbo].[AbuseLogs]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
