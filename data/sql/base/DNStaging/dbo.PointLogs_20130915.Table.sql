USE [dnstaging]
GO
/****** Object:  Table [dbo].[PointLogs_20130915]    Script Date: 11/14/2026 9:00:08 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[PointLogs_20130915](
	[LogDate] [datetime2](0) NOT NULL,
	[CharacterID] [bigint] NOT NULL,
	[PointCode] [tinyint] NOT NULL,
	[PointBefore] [int] NOT NULL,
	[PointAfter] [int] NOT NULL,
	[MapID] [int] NOT NULL,
	[IP] [int] NOT NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_PointLogs_20130915_LogDate]    Script Date: 11/14/2026 9:00:08 AM ******/
CREATE CLUSTERED INDEX [IX_NC_PointLogs_20130915_LogDate] ON [dbo].[PointLogs_20130915]
(
	[LogDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_PointLogs_20130915_CharacterID]    Script Date: 11/14/2026 9:01:05 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_PointLogs_20130915_CharacterID] ON [dbo].[PointLogs_20130915]
(
	[CharacterID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[PointLogs_20130915]  WITH CHECK ADD  CONSTRAINT [CK_PointLogs_20130915_LogDate] CHECK  (([LogDate]>='2013-09-15' AND [LogDate]<'2013-09-16'))
GO
ALTER TABLE [dbo].[PointLogs_20130915] CHECK CONSTRAINT [CK_PointLogs_20130915_LogDate]
GO
