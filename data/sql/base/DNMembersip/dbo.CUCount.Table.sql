USE [dnmembership]
GO
/****** Object:  Table [dbo].[CUCount]    Script Date: 11/14/2026 8:51:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[CUCount]') AND type in (N'U'))
BEGIN
CREATE TABLE [dbo].[CUCount](
	[SnapshotDate] [datetime2](0) NOT NULL,
	[WorldID] [int] NOT NULL,
	[CUCount] [int] NOT NULL,
	[ChannelPartnerCode] [tinyint] NOT NULL,
 CONSTRAINT [PK_CUCount] PRIMARY KEY CLUSTERED 
(
	[SnapshotDate] ASC,
	[WorldID] ASC,
	[ChannelPartnerCode] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [DataFG1]
) ON [DataFG1]
END
GO
IF NOT EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[DF_CUCount_ChannelPartnerCode]') AND type = 'D')
BEGIN
ALTER TABLE [dbo].[CUCount] ADD  CONSTRAINT [DF_CUCount_ChannelPartnerCode]  DEFAULT ((0)) FOR [ChannelPartnerCode]
END
GO
