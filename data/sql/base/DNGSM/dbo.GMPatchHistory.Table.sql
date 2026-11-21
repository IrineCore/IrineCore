USE [dngsm]
GO
/****** Object:  Table [dbo].[GMPatchHistory]    Script Date: 11/14/2026 8:47:03 AM ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[GMPatchHistory](
	[PatchVersion] [nvarchar](30) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[PatchDate] [datetime] NOT NULL,
	[GMID] [int] NOT NULL,
	[PatchMemo] [nvarchar](200) COLLATE SQL_Latin1_General_CP1_CI_AS NULL
) ON [PRIMARY]
GO
/****** Object:  Index [IX_NC_GMPatchHistory_PatchDate]    Script Date: 11/14/2026 8:47:03 AM ******/
CREATE CLUSTERED INDEX [IX_NC_GMPatchHistory_PatchDate] ON [dbo].[GMPatchHistory]
(
	[PatchDate] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
/****** Object:  Index [IX_NN_GMPatchHistory_GMID]    Script Date: 11/14/2026 8:47:09 AM ******/
CREATE NONCLUSTERED INDEX [IX_NN_GMPatchHistory_GMID] ON [dbo].[GMPatchHistory]
(
	[GMID] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
