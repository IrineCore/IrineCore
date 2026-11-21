USE [dnworld]
GO
/****** Object:  Table [dbo].[AbuseMonitor]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[AbuseMonitor](
	[CharacterID] [bigint] NOT NULL,
	[AbuseCount1] [int] NOT NULL,
	[AbuseCount2] [int] NOT NULL,
	[PlayRestraint] [int] NULL,
	[ResetRestraint] [int] NULL,
 CONSTRAINT [PK_AbuseMonitor] PRIMARY KEY CLUSTERED 
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_NNI_AbuseMonitor_AbuseCount1]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_AbuseMonitor_AbuseCount1] ON [dbo].[AbuseMonitor]
(
	[AbuseCount1] ASC
)
INCLUDE([AbuseCount2]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
/****** Object:  Index [IX_NNI_AbuseMonitor_AbuseCount2]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE NONCLUSTERED INDEX [IX_NNI_AbuseMonitor_AbuseCount2] ON [dbo].[AbuseMonitor]
(
	[AbuseCount2] ASC
)
INCLUDE([AbuseCount1]) WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [IndexFG1]
GO
