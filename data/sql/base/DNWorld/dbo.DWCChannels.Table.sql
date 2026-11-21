USE [dnworld]
GO
/****** Object:  Table [dbo].[DWCChannels]    Script Date: 11/14/2026 8:54:15 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[DWCChannels](
	[DWCSequence] [int] NOT NULL,
	[DWCStartTime] [datetime2](0) NOT NULL,
	[DWCEndTime] [datetime2](0) NOT NULL,
	[EnableFlag] [bit] NOT NULL,
	[StatusCode] [tinyint] NOT NULL,
	[LadderTime1] [int] NULL,
	[LadderTime2] [int] NULL,
	[DWCStopTime] [datetime2](0) NULL,
	[DWCReopenTime] [datetime2](0) NULL,
	[LimitCharacterLevel] [tinyint] NULL,
	[DataInitFlag] [bit] NOT NULL,
 CONSTRAINT [PK_DWCChannels] PRIMARY KEY CLUSTERED 
(
	[DWCSequence] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
GO
/****** Object:  Index [IX_UNF_DWCChannels_EnableFlag]    Script Date: 11/14/2026 8:54:37 AM ******/
CREATE UNIQUE NONCLUSTERED INDEX [IX_UNF_DWCChannels_EnableFlag] ON [dbo].[DWCChannels]
(
	[EnableFlag] ASC
)
WHERE ([EnableFlag]=(1))
WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
GO
ALTER TABLE [dbo].[DWCChannels] ADD  CONSTRAINT [DF_DWCChannels_StatusCode]  DEFAULT ((1)) FOR [StatusCode]
GO
ALTER TABLE [dbo].[DWCChannels] ADD  CONSTRAINT [DF_DWCChannels_DataInitFlag]  DEFAULT ((0)) FOR [DataInitFlag]
GO
