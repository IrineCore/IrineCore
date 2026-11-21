USE [dnworld]
GO
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
CREATE TABLE [dbo].[BoardAttachment](
	[AttachmentSeq] [int] IDENTITY(1,1) NOT NULL,
	[BoardSeq] [int] NOT NULL,
	[RealFileName] [nvarchar](100) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[ChangedFileName] [nvarchar](100) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[FileDescription] [nvarchar](200) COLLATE SQL_Latin1_General_CP1_CI_AS NOT NULL,
	[AttachementDate] [datetime] NOT NULL
) ON [PRIMARY]
GO
CREATE CLUSTERED INDEX [CIX_BoardAttachment_BoardSeq] ON [dbo].[BoardAttachment]
(
	[BoardSeq] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, DROP_EXISTING = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[BoardAttachment] ADD CONSTRAINT [PK_BoardAttachment] PRIMARY KEY NONCLUSTERED
(
	[AttachmentSeq] ASC
)WITH (PAD_INDEX = OFF, STATISTICS_NORECOMPUTE = OFF, SORT_IN_TEMPDB = OFF, IGNORE_DUP_KEY = OFF, ONLINE = OFF, ALLOW_ROW_LOCKS = ON, ALLOW_PAGE_LOCKS = ON) ON [PRIMARY]
GO
ALTER TABLE [dbo].[BoardAttachment] WITH CHECK ADD CONSTRAINT [FK_BoardAttachment_BoardMain] FOREIGN KEY([BoardSeq])
REFERENCES [dbo].[BoardMain] ([BoardSeq])
GO
ALTER TABLE [dbo].[BoardAttachment] CHECK CONSTRAINT [FK_BoardAttachment_BoardMain]
GO
